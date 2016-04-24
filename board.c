#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "serverhelper.h"

#define COMMAND 1
#define INTLENGTH 4

int main(int argc, char *argv[]){
	int pidfd, dirfd;
	pid_t pid;
	int channelid;
	int msgsize;
	int bytesleft;
	int templength, templength2;
	int b1fd, b2fd, p1fd, p2fd;
	int b1bytes, b2bytes, p1bytes, p2bytes;
	char msgbuf;
	int i;
	char *text;
	DIR *dir_ptr;
	struct stat st = {0};

	//	Check if the arguments number is valid
	if (argc != 2) {
		printf("Usage: Provide Path Name \n");
		exit(1);
	}

	//	Check if the given path exists, else create it
	if (stat(argv[1], &st) == -1) {
		mkdir(argv[1], 0777);
	}
	if ((dir_ptr = opendir(argv[1])) == NULL) {
		fprintf(stderr, "cannot open or create %s\n", argv[1]);
		exit(1);
	}

	//	Check if the _pid file exists
	//	If not create it, add the child pid and run the server
	if ((dirfd = open("/tmp/sdi1100094/_pid", O_RDONLY)) == -1) {
		dirfd = creat("/tmp/sdi1100094/_pid", 0777);
		pid = fork();
		if (pid == -1) {
			perror("Fork failed");
			exit(1);
		}
		if (pid != 0) {
			pidfd = open("/tmp/sdi1100094/_pid", O_RDONLY);
			char temp[20];
			sprintf(temp, "%d", pidfd);
			write(pidfd, temp, 20);
			close(pidfd);
		}

		//	Server
		if (pid == 0) {
			printf("Process pid = %d\n", getppid());
			//	Create all the required named pipes
			if ( mkfifo("/tmp/sdi1100094/_boardfifo1", 0666) == -1 ){
				if ( errno != EEXIST ) {
					perror("receiver: boardfifo1");
					exit(6); };
			}
			if ( mkfifo("/tmp/sdi1100094/_boardfifo2", 0666) == -1 ){
				if ( errno != EEXIST ) {
					perror("receiver: boardfifo2");
					exit(6); };
			}
			if ( mkfifo("/tmp/sdi1100094/_postfifo1", 0666) == -1 ){
				if ( errno != EEXIST ) {
					perror("receiver: postfifo1");
					exit(6); };
			}
			if ( mkfifo("/tmp/sdi1100094/_postfifo2", 0666) == -1 ){
				if ( errno != EEXIST ) {
					perror("receiver: postfifo2");
					exit(6); };
			}

			int validch = 0;
			int arraysize;
			int numchannels;
			channel *channarray;

			channarray = malloc(4*sizeof(channel));
			for (i = 0; i < 4; i++) {
				channarray[i].id = 0;
				channarray[i].numMess = 0;
			}
			arraysize = 4;
			numchannels = 0;

			if ( (b1fd = open("/tmp/sdi1100094/_boardfifo1", O_RDONLY | O_NONBLOCK)) < 0){
				perror("boardfifo1 open problem");
				exit(3);
				}
			if ( (p1fd = open("/tmp/sdi1100094/_postfifo1", O_RDONLY | O_NONBLOCK)) < 0){
				perror("postfifo1 open problem");
				exit(3);
				}

			//	Non blocking read until a client or boarpost connects
			for (;;) {
				if ( (b1bytes = read(b1fd, &msgbuf, COMMAND)) == 1) {
					if ( (b2fd = open("/tmp/sdi1100094/_boardfifo2", O_WRONLY)) < 0){
						perror("boardfifo2 open problem");
						exit(3);
					}

					if (msgbuf == 'c') {
						if ( (read(b1fd, &channelid, INTLENGTH)) == 4) {
							printf("message from boardpost received = %d\n", channelid);
						}
						if (checkChannel(channarray, numchannels, channelid) == 0) {
							validch = 1;
						}

						if ( (read(b1fd, &msgsize, INTLENGTH)) == 4) {
							printf("message from boardpost received = %d\n", msgsize);
						}
						text = malloc(msgsize);

						if (validch) {
							//	Check if the channel array is full
							if (numchannels == arraysize) {
								arraysize *= 2;
								channarray = realloc(channarray, arraysize*sizeof(channel));
							}

							channarray[numchannels].id = channelid;
							channarray[numchannels].numMess = 0;
							channarray[numchannels].name = malloc(msgsize);
							channarray[numchannels].messages = malloc(4*sizeof(message));
							numchannels++;
						}

						bytesleft = msgsize;
						while ((b1bytes = read(b1fd, text, msgsize)) > bytesleft) {
							if (b1bytes > 0) {
								bytesleft -= b1bytes;
							}
							if (validch && b1bytes > 0) {
								memcpy(channarray[numchannels - 1].name + strlen(channarray[numchannels - 1].name), text, b1bytes);
							}
						}
						if (validch) {
							memcpy(channarray[numchannels - 1].name + strlen(channarray[numchannels - 1].name), text, b1bytes);
						}

						if (write(b2fd, &validch, INTLENGTH) == -1) {
								perror("Error in Writing (server channel id)");
								exit(2);
						}

						validch = 0;
						free(text);
					}

					if (msgbuf == 'g') {
					}

					if (msgbuf == 's') {
						close(b2fd);
						break;
					}

					close(b2fd);
				}

				if ( (read(p1fd, &msgbuf, COMMAND)) == 1) {
					if ( (p2fd = open("/tmp/sdi1100094/_postfifo2", O_WRONLY)) < 0){
						perror("postfifo2 open problem");
						exit(3);
					}

					if (msgbuf == 'l') {
						if (write(p2fd, &numchannels, INTLENGTH) == -1) {
								perror("Error in Writing (server list)");
								exit(2);
						}

						for (i = 0; i < numchannels; i++) {
							templength = channarray[i].id;
							templength2 = strlen(channarray[i].name);
							// printf("length = %d\n", templength2);

							if (write(p2fd, &templength, INTLENGTH) == -1) {
									perror("Error in Writing (server list)");
									exit(2);
							}
							// printf("passed this\n");
							if (write(p2fd, &templength2, INTLENGTH) == -1) {
									perror("Error in Writing (server list)");
									exit(2);
							}
							// printf("and this\n");
							text = channarray[i].name;
							if (write(p2fd, text, templength2) == -1) {
									perror("Error in Writing (server list)");
									exit(2);
							}
						}
					}

					if (msgbuf == 'w') {
						printf("message from boardpost received = %c\n", msgbuf);
						if ( (read(p1fd, &channelid, INTLENGTH)) == 4) {
							printf("message from boardpost received = %d\n", channelid);
						}
						if ( (read(p1fd, &msgsize, INTLENGTH)) == 4) {
							printf("message from boardpost received = %d\n", msgsize);
						}
						text = malloc(msgsize);

						bytesleft = msgsize;
						while ((p1bytes = read(p1fd, text, msgsize)) > bytesleft) {
							if (p1bytes > 0) {
								bytesleft -= p1bytes;
							}
						}
						printf("message in server = %s\n", text);
						free(text);
					}

					close(p2fd);
				}
			}
			// close(b1fd);
			// close(p1fd);
			exit(0);
		}
	}

	//	Make sure all pipes are created and opened
	usleep(200000);

	const char s[2] = " ";
	char *token;
	char command;
	int nwrite;
	int tempbuf;
	char str[300];

	if ( (b1fd = open("/tmp/sdi1100094/_boardfifo1", O_WRONLY)) < 0) {
			perror("fifo open error");
			exit(1);
	}
	if ( (b2fd = open("/tmp/sdi1100094/_boardfifo2", O_RDONLY | O_NONBLOCK)) < 0) {
			perror("fifo open error");
			exit(1);
	}

	//	ToDo open boardfifo2 for validation
	while (fgets(str, sizeof(str), stdin) != NULL) {
		if (str[strlen(str) - 1] == '\n') {
			str[strlen(str) - 1] = '\0';
		}

		token = strtok(str, s);

		if (strcmp("createchannel", token) == 0) {
			command = 'c';
			if ((nwrite = write(b1fd, &command, 1)) == -1) {
					perror("Error in Writing (channel)");
					exit(2);
			}
			token = strtok(NULL, s);
			channelid = atoi(token);

			if ((nwrite = write(b1fd, &channelid, INTLENGTH)) == -1) {
					perror("Error in Writing (channel id)");
					exit(2);
			}
			token = strtok(NULL, "");
			templength = strlen(token);
			if ((nwrite = write(b1fd, &templength, INTLENGTH)) == -1) {
					perror("Error in Writing (channel message length)");
					exit(2);
			}

			if ((nwrite = write(b1fd, token, templength)) == -1) {
					perror("Error in Writing (channel text)");
					exit(2);
			}

			while (read(b2fd,&tempbuf,INTLENGTH) < 1);
			if (tempbuf == 1) {
				printf("Channel with ID %d created successfully\n", channelid);
			}
			else {
				printf("Error creating the channel with ID %d\n", channelid);
			}
		}
		else if (strcmp("getmessages", token) == 0) {
			command = 'w';
			if ((nwrite = write(b1fd, &command, COMMAND)) == -1) {
					perror("Error in Writing (write)");
					exit(2);
			}
			token = strtok(NULL, s);
			channelid = atoi(token);
			// printf("channelid = %d\n", channelid);
			if ((nwrite = write(b1fd, &channelid, INTLENGTH)) == -1) {
					perror("Error in Writing (write channel)");
					exit(2);
			}
			token = strtok(NULL, "");
			templength = strlen(token);
			if ((nwrite = write(b1fd, &templength, INTLENGTH)) == -1) {
					perror("Error in Writing (write message length)");
					exit(2);
			}
			// printf("message %s with size %d\n", token, templength);
			if ((nwrite = write(b1fd, token, templength)) == -1) {
					perror("Error in Writing (write channel)");
					exit(2);
			}
		}
		else if (strcmp("exit", token) == 0) {
			break;
		}
		else if (strcmp("shutdown", token) == 0) {
			command = 's';
			if ((nwrite = write(b1fd, &command, 1)) == -1) {
					perror("Error in Writing (channel)");
					exit(2);
			}
		}
		else {
			printf("Wrong command\n");
		}
	}

	exit(0);
}