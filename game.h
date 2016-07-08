/*
Kurt Stoeckl kstoeckl
*/

#include <netinet/in.h>

#define LENCODE 4
#define LENRESULTS 8
#define LENGTHEND 8
#define FLAG 'z'
#define SMALLESTVALUE 'A'
#define LARGESTVALUE 'F'
#define VALID 1
#define INVALID 0
#define TURNLIMIT 10
#define NOTDONE 0
#define SUCCESS 1
#define FAILURE -1


typedef struct{
	int socket;
	int turn;
	int thread_index;
	char secret_code[LENCODE+1];
	char ip4[INET_ADDRSTRLEN];
}game;

//Increments the turn, makes the guess, gives the results of the guess
int updateGameState(game* g,char guess[],char results[]);
//Validates that the code contains characters in the correct range
int validateCode(char code[]);
//Generates a random code
void generateRandomCode(char* code);