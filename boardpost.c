#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <error.h>
#include <unistd.h>

#define COMMAND 1
#define INTLENGTH 4

int main(int argc, char *argv[]){
	const char s[2] = " ";
	char *token;
	char *name;
	char *temptext;
	char command;
	int chanelnum;
	int channelid;
	int templength, templength2;
	int p1fd, p2fd;
	int p1bytes, p2bytes;
	int bytesleft;
	int i;
	int nwrite;
	char str[300];

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
				// if (read(p2fd, &channelid, INTLENGTH) == 4) {
				// 	printf("Channel %d ID = %d\n", i, channelid);
				// }
				// printf("AAAA\n");
				// if (read(p2fd, &templength2, INTLENGTH) == 4) {
				// 	printf("BBBB\n");
				// 	temptext = malloc(templength2);
				// 	name = malloc(templength2);
				// 	printf("CCCC\n");
				// }
				//
				// while ((p2bytes = read(p2fd, temptext, templength2)) > bytesleft) {
				// 	if (p2bytes < 0) {
				// 		printf("DDDD\n");
				// 	}
				// 	if (p2bytes > 0) {
				// 		bytesleft -= p2bytes;
				// 		memcpy(name + strlen(name), temptext, p2bytes);
				// 	}
				// }
				// printf("DDDDDD\n");
				// memcpy(name + strlen(name), temptext, p2bytes);
				//
				// printf("Channel name = %s\n", name);
				printf("Channel %d\n", i+1);
				if ( (read(p2fd, &channelid, INTLENGTH)) == 4) {
					printf("ID = %d\n", channelid);
				}
				read(p2fd, &templength2, INTLENGTH);

				temptext = malloc(templength2);
				name = malloc(templength2);

				bytesleft = templength2;
				while ((p2bytes = read(p2fd, temptext, templength2)) > bytesleft) {
					if (p2bytes > 0) {
						bytesleft -= p2bytes;
						memcpy(name + strlen(name), temptext, p2bytes);
					}
				}
				memcpy(name + strlen(name), temptext, p2bytes);
				printf("Name = %s\n", name);

				free(temptext);
				free(name);
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
		}
		else if (strcmp("send", token) == 0) {
		}
		else {
			printf("Wrong command\n");
		}
  }

	exit(0);
}
