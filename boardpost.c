#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <libgen.h>

#define COMMAND 1
#define INTLENGTH 4
#define FILEREAD 1024

int main(int argc, char *argv[]){
	const char s[2] = " ";
	char *token;
	char *temptext;
	char *filename;
	char command;
	int fd;
	int bytesread;
	int filesize;
	int chanelnum;
	int channelid;
	int templength, templength2;
	int p1fd, p2fd;
	int p1bytes, p2bytes;
	int bytesleft;
	int i;
	int nwrite;
	char str[300];

	struct stat statbuf;

	if (argc != 2) {
		printf("Usage: Provide Path Name\n");
		exit(1);
	}

	if ( (p1fd = open("/tmp/sdi1100094/_postfifo1", O_WRONLY)) < 0) {
			perror("fifo open error");
			exit(1);
	}
	if ( (p2fd = open("/tmp/sdi1100094/_postfifo2", O_RDONLY | O_NONBLOCK)) < 0) {
			perror("fifo open error");
			exit(1);
	}

	while (fgets(str, sizeof(str), stdin) != NULL) {
		if (str[strlen(str) - 1] == '\n') {
      str[strlen(str) - 1] = '\0';
		}

		token = strtok(str, s);

		if (strcmp("list", token) == 0) {
			command = 'l';
			if ((nwrite = write(p1fd, &command, 1)) == -1) {
					perror("Error in Writing (list)");
					exit(2);
			}
			while (read(p2fd,&templength,INTLENGTH) < 1);
			if (templength == 0) {
				printf("No channels to show yet\n");
				continue;
			}

			printf("templength = %d\n", templength);
			for (i = 0; i < templength; i++) {
				printf("Channel %d\n", i+1);
				if ( (read(p2fd, &channelid, INTLENGTH)) == 4) {
					printf("ID = %d\n", channelid);
				}
				read(p2fd, &templength2, INTLENGTH);

				temptext = malloc(templength2 + 1);

				// bytesleft = templength2;
				// while ((p2bytes = read(p2fd, temptext, templength2)) > bytesleft) {
				// 	if (p2bytes > 0) {
				// 		bytesleft -= p2bytes;
				// 		memcpy(name + strlen(name), temptext, p2bytes);
				// 	}
				// }
				// memcpy(name + strlen(name), temptext, p2bytes);


				bytesleft = templength2;
				while ((p2bytes = read(p2fd, temptext, templength2)) > bytesleft) {
					if (p2bytes > 0) {
						temptext += p2bytes;
						bytesleft -= p2bytes;
					}
				}
				temptext += p2bytes;
				*temptext = '\0';
				temptext -= templength2;


				printf("Name = %s\n", temptext);

				free(temptext);
			}
		}
		else if (strcmp("write", token) == 0) {
			command = 'w';
			if ((nwrite = write(p1fd, &command, COMMAND)) == -1) {
					perror("Error in Writing (write)");
					exit(2);
			}
			token = strtok(NULL, s);
			chanelnum = atoi(token);
			// printf("chanelnum = %d\n", chanelnum);
			if ((nwrite = write(p1fd, &chanelnum, INTLENGTH)) == -1) {
					perror("Error in Writing (write chanel)");
					exit(2);
			}
			token = strtok(NULL, "");
			templength = strlen(token);
			if ((nwrite = write(p1fd, &templength, INTLENGTH)) == -1) {
					perror("Error in Writing (write message length)");
					exit(2);
			}
			// printf("message %s with size %d\n", token, templength);
			if ((nwrite = write(p1fd, token, templength)) == -1) {
					perror("Error in Writing (write text)");
					exit(2);
			}

			while (read(p2fd,&templength,INTLENGTH) < 1);

			if (templength == 1) {
				printf("Data stored successfully\n");
			}
			else {
				printf("Unable to locate that channel\n");
			}
		}
		else if (strcmp("send", token) == 0) {
			command = 's';

			if ((nwrite = write(p1fd, &command, COMMAND)) == -1) {
					perror("Error in Writing (send)");
					exit(2);
			}
			token = strtok(NULL, s);
			chanelnum = atoi(token);

			if ((nwrite = write(p1fd, &chanelnum, INTLENGTH)) == -1) {
					perror("Error in Writing (send chanel)");
					exit(2);
			}
			token = strtok(NULL, "");
			templength = strlen(token);

			if ((nwrite = write(p1fd, &templength, INTLENGTH)) == -1) {
					perror("Error in Writing (send message length)");
					exit(2);
			}
			if ((nwrite = write(p1fd, token, templength)) == -1) {
					perror("Error in Writing (send text)");
					exit(2);
			}

			if ((fd = open(token, O_RDONLY, 0777)) == -1) {
				printf("File does not exist\n");
				continue;
			}

			if (fstat(fd, &statbuf) == -1) {
				perror("Failed to get file status");
				exit(2);
			}
			else {
				filesize = (int)statbuf.st_size;
			}

			if ((nwrite = write(p1fd, &filesize, INTLENGTH)) == -1) {
					perror("Error in Writing (send filesize)");
					exit(2);
			}

			temptext = malloc(FILEREAD);
			bytesread = FILEREAD;

			while (filesize > 0) {
				if (filesize < FILEREAD) {
					bytesread = filesize;
				}
				if ((p1bytes = read(fd, temptext, bytesread)) > 0 ) {
					nwrite = write(p1fd, temptext, p1bytes);
					filesize -= p1bytes;
				}
			}

			while (read(p2fd, &templength, INTLENGTH) < 1);
			if (templength) {
				printf("File was sent successfully\n");
			}
			// filename = basename(token);
			// printf("filename = %s\n", filename);
			close(fd);
			free(temptext);
		}
		else {
			printf("Wrong command\n");
		}
  }

	exit(0);
}
