// AUTHOR: Alon Leshem
// all296@pitt.edu

#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

// ERRORS & RETURNS ------
#define ERR_NO_ARG -1
#define ERR_NO_RECEIVER -2
#define ERR_NO_MSG -3
#define SUCCESS 0
#define NO_MSG -1
#define LAST_MSG 0
#define MORE_MSG 1
// CONSTANTS -------------
#define LEN_FROM 16
#define LEN_MSG 256

// sys_cs1501_send_msg as C function
int send_msg(char *to, char *msg, char *from) {
	syscall(441, to, msg, from);
}

// sys_cs1501_get_msg as C function
int get_msg(char *to, char *msg, long msg_len, char *from, long from_len) {
	syscall(442, to, msg, msg_len, from, from_len);
}

int read_helper(char *to) {
	char from[LEN_FROM];
	char msg[LEN_MSG];
	int counter = 0;
	int status = get_msg(to, msg, LEN_MSG, from, LEN_FROM);

	if(status == NO_MSG) {
		return printf("No messages for %s\n", to);
	}

	// can be consolidated in the loop but
	// I wanted to maintain a visual difference
	// between 1 msg and n messages
	if(status == LAST_MSG) {
		return printf("%s: \"%s\"\n", from, msg);
	}

	/*
	if there are multiple messages queued,
		print out msg # to differentiate messages

	assumption: there will be less than 100 msg s
		(for formatting purposes only, still works
		but output will not line up as nicely)
	
	will keep looping as long as at least 1 message
		(i.e. status = 0 || 1)
	*/
	while(status > NO_MSG) {
		++counter;
		printf("%2d. %s: \"%s\"\n", counter, from, msg);
		status = get_msg(to, msg, LEN_MSG, from, LEN_FROM);
	}
	return status;
}

int main(int argc, char** argv) {
	// no argument / fake argument -- print generic usage
	if(argc == 1 || (strncmp(argv[1], "-r", 2) && strncmp(argv[1], "-s", 2))) {
		fprintf(stderr, "usage: %s [-r] [-s to_user msg]\n", argv[0]);
		return ERR_NO_ARG;
	}

	// -s with no <to_user>
	if(argc <= 2 && !strncmp(argv[1], "-s", 2)) {
		fprintf(stderr, "usage: %s [-r] [-s to_user msg] (must specify target user and message)\n", argv[0]);
		return ERR_NO_RECEIVER;
	}

	// -s with no <msg>
	if(argc <= 3 && !strncmp(argv[1], "-s", 2)) {
		fprintf(stderr, "usage: %s [-r] [-s to_user msg] (must include a message to send)\n", argv[0]);
		return ERR_NO_MSG;
	}

	// get $USER
	char *user = getenv("USER");
	
	// send goes straight to syscall
	if(!strncmp(argv[1], "-s", 2)) {

		return send_msg(argv[2], argv[3], user);
	}

	// read goes to helper for processing
	if(!strncmp(argv[1], "-r", 2)) {
		return read_helper(user);
	}

	return SUCCESS;
}
