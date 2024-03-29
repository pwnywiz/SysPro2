#ifndef SERVERHELPER_H_
#define SERVERHELPER_H_

struct message {
	int isMessage;	//	is it a file or a message
	char *text;			//	text of the message or path of the file
};

typedef struct message message;

struct channel {
	int id;
	char *name;		//	Name of the channel
	int numMess;	//	Number of messages in the channel
	int numArray; //	Number of empty array slots
	message *messages;	//	Messages array
};

typedef struct channel channel;

int checkChannel(channel *chann, int num, int id);

#endif /* SERVERHELPER_H_ */
