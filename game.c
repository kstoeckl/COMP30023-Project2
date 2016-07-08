/*
Kurt Stoeckl kstoeckl
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
/*
Increments the turn, makes the guess, gives the results of the guess
game_state: a pointer to a record of the state of the game
guess: A string of length LENCODE, representing the clients last guess
results: A string representing the result of the turn
returns: 1 if SUCCESS, 0 if Not Done, -1 if FAILURE
*/
int updateGameState(game* g,char guess[],char *results){
	int complete_match=0,partial_match=0;
	int i,j;
	char* code_copy = (char*)malloc(sizeof(char)*LENCODE);

	memcpy(code_copy,g->secret_code,LENCODE);

	g->turn++;

	int check = validateCode(guess);
	
	for (i=0;i<4;i++){
		if (g->secret_code[i]==guess[i]){
			complete_match+=1;
			//flags to stop the complete match being included
			// in the partial match count
			code_copy[i]=FLAG;
			guess[i]=FLAG;
		}
	}
	for (i=0;i<4;i++){
		for (j=0;j<4;j++){
			if(i==j) j++;
			if (code_copy[j]==guess[i] && code_copy[j]!=FLAG){
				partial_match+=1;
				code_copy[j]=FLAG;
				break;
			}
		}
	}	
	if(complete_match==LENCODE){
		sprintf(results,"SUCCESS");
		return SUCCESS;
	}
	else if (g->turn==TURNLIMIT){
		sprintf(results,"FAILURE");
		return FAILURE;
	}
	else if (check==INVALID){
		sprintf(results,"INVALID");	
	}
	else{		
		sprintf(results,"[%d,%d]",complete_match,partial_match);
	}
	return NOTDONE;
}
/*
Validates that the code contains characters in the correct range
code: the code being checked
returns: 1 if code is valid, 0 if not
*/
int validateCode(char code[]){
	int i;
	for (i=0;i<LENCODE;i++){
		if (!(code[i]>=SMALLESTVALUE && code[i]<=LARGESTVALUE)){
			return INVALID;
		}
	}
	return VALID;
}
/*
Generates a random code
code: a passed pointer that is set to point at a random code
*/
void generateRandomCode(char* code){
	int i;
	//generate new seed based off time
	srand(time(NULL));
	for (i=0;i<LENCODE;i++){
		*(code+i) = rand()%(LARGESTVALUE+1-SMALLESTVALUE)+SMALLESTVALUE;
	}
}