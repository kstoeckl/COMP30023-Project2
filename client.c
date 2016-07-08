/*
Kurt Stoeckl kstoeckl
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "game.h"

#define SIZEWMSG 100

int main(int argc, char * argv[]){
	char code_guess[LENCODE];
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;	
	int s, server_port, guess_count=1;	

	if(argc==3){
		host = argv[1];
		server_port = atoi(argv[2]);
	}
	else {
		fprintf(stderr, "Usage :client host server_port\n");
		exit(1);
	}

	/* translate host name into peer's IP address ; This is name translation service by the operating system */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "Unknown host %s  \n",host);
		exit(1);
	}
	/* Building data structures for socket */

	bzero( (char *)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port =htons(server_port);

	/* Active open */
	/* Preliminary steps: Setup: creation of active open socket*/

	if ( (s = socket(AF_INET, SOCK_STREAM, 0 )) < 0 ){
		perror("Error in creating a socket to the server");
		exit(1);
	}

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin))  < 0  ){
		perror("Error in connecting to the host");
		close(s);
		exit(1);
	}

	char* weclomemsg=(char*)malloc(sizeof(char)*SIZEWMSG);
	char* resultmsg = (char*)malloc(sizeof(char)*(LENRESULTS));
	recv(s,weclomemsg,SIZEWMSG,0);
	printf("%s",weclomemsg);

	while(1){
		printf("Guess %d >>",guess_count);
		scanf("%s",code_guess);
		send(s,code_guess,LENCODE,0);
		if (recv(s,resultmsg,LENRESULTS,0))
			printf("%s\n",resultmsg);
		if (*resultmsg=='S' || *resultmsg=='F')
			exit(0);
		guess_count++;
	}
}