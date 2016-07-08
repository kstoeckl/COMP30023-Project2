/*
Kurt Stoeckl kstoeckl
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "game.h"

#define MAXTHREADS 20
#define BUFFERSIZE 100
#define MICROSEC 1000000

void* main_thread(void *arg);
void* serve_game(void* socket);
void write_to_log(char* string);
void SIGINT_handler(int sig);
void get_time(char** time_string);
void retrieve_performance_statistics();

const static char* perf_log = "log.txt";
const static char* stat_path = "/proc/self/statm";
const static char WELCOMEMSG[] = "Welcome to MASTERMIND\n"\
	"For a complete overview of the rules see the Wikipedia Entry...\n";

pthread_mutex_t lock;
FILE* logfp ;

//Performance Variables
int thread_count=0,connect_count=0;
int success_count=0;
int user_usecs=0,system_usecs=0;

pthread_t main_worker_thread;
pthread_t threads[MAXTHREADS];
game thread_args[MAXTHREADS];

typedef struct{
	int socket;
	struct sockaddr_in server, client;
	char secret_code[LENCODE];
}arg_struct;

int main (int argc, char *argv[]){
	struct sockaddr_in server, client;
	int s, server_port;
	arg_struct arg_passer;
	char secret_code[LENCODE];

	//register the ctr-C with an appropriate handler
	signal(SIGINT,SIGINT_handler);

	//removes the performance log if it already exists
	remove(perf_log);

	if (argc<2 || argc>3) {
		fprintf(stderr, "Usage :server portnumber\n");
		exit(1);
	}
	if(argc==3){
		memcpy(secret_code,argv[2],LENCODE);
	}
	else{
		secret_code[0] = FLAG;
	}
	server_port = atoi(argv[1]);	
	printf("Server port %i\n",server_port);
	/* Building data structures for sockets */
	/* Identify two end points; one for the server and the other
	 for the client when it connects to the server socket */
	memset (&server,0, sizeof (server));
	memset (&client,0, sizeof (client));

	/* AF_INET: specifies the connection to Internet.
	INADDR_ANY specifies the server is willing to accept 
	connections on any of the local host's IP addresses. */ 

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons (server_port); 

	/*Creation of passive open socket*/
	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0){
      perror ("Error creating socket");
      exit (1);
	}
	printf("Socket descriptor:  %d\n", s);

	/* Bind the socket to local address */

	if (bind (s, (struct sockaddr *) &server, sizeof (server)) < 0){
      perror ("Error in binding to the specified port");
      exit (1);
	}
	printf("sock:  family = %d\n", server.sin_family);
	printf("       saddr  = %d\n", server.sin_addr.s_addr);
	printf("       port   = %d\n", ntohs(server.sin_port));

	/* Sets the maximum number of pending connections to be allowed,
	in our case this number is 10 */
	if ( listen (s, 10) < 0){
        perror("listen() failed with error");
        exit(1);
	}
	else{
		printf("Listening on port %d...\n", ntohs(server.sin_port));
	}

	arg_passer.socket = s;
	arg_passer.client = client;
	memcpy(arg_passer.secret_code,secret_code,LENCODE);

	//Creates a thread to handle clients
	if (pthread_create(&main_worker_thread,NULL,main_thread,(void *)&arg_passer)){
		fprintf(stderr,"Failed to create Thread\n");
		exit(1);
	}
	//waits until the main_worker_thread has been canceled (due to ctr-C)
	pthread_join(main_worker_thread,0);

	//Generates and appends final report to the log file.

	char* final_report = (char*)malloc(sizeof(char)*BUFFERSIZE);	
	struct rusage usage;

	getrusage(RUSAGE_SELF, &usage);	

	sprintf(final_report,"Connections:%d, Successes:%d,"\
	 "UserTime:%ld.%06ld, SystemTime:%ld.%06ld\n",
		connect_count,success_count, 
		usage.ru_utime.tv_sec,usage.ru_utime.tv_usec,
		usage.ru_stime.tv_sec,usage.ru_stime.tv_usec);
	
	write_to_log("Killed with ctr-C\n");
	write_to_log(final_report);

	retrieve_performance_statistics();

	close(s);
	return 1;
}
/*
This thread waits on the arrival of clients and creates new threads
to serve the game to those clients.
arg: A struct that contains:
	The Server's socket number
	The IP address of the server
	And the Secret Code passed as an arg to server (or if none
	was passed, a char* containing a flag which indicates so.)
*/
void* main_thread(void *arg){
	int new_s,s;
	socklen_t len;	
	struct sockaddr_in  client;	
	char secret_code[LENCODE];
	char* log_line = (char*)malloc(sizeof(char)*BUFFERSIZE);
	char* buffer = (char*)malloc(sizeof(char)*BUFFERSIZE);

	arg_struct *args = arg;
	client = args->client;
	s = args->socket;
	memcpy(secret_code,args->secret_code,LENCODE);

	while (1){
		len=sizeof(client);
		if ((new_s = accept (s, (struct sockaddr *) &client, &len)) < 0){
			printf("errno = %d, connect_count =%d, new_s = %d\n",
			 errno, connect_count, new_s);
			perror ("Accept failed");
			exit (1);
		}
		else{
			//drops a connections and sends busy message if at client limit
			if(thread_count>=MAXTHREADS){
				send(new_s,"Busy.\n",7,0);
				close(new_s);
				continue;
			}
			++connect_count;

			char ip4[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&(client.sin_addr), ip4, INET_ADDRSTRLEN);
			printf("connection accepted from client %s\n",ip4);

			get_time(&buffer);
			sprintf(log_line,"<%s>(%s)(Soc_ID %d) connection accepted from client \n",
				buffer,ip4,new_s);
			write_to_log(log_line);

			//Places the thread in the first empty slot of the thread array
			int i,empty;
			for(i=0;i<MAXTHREADS;i++){
				//printf("%d\n",threads[i]);
				if (threads[i]==0){
					empty = i;
					break;
				}
			}
			thread_args[empty].socket = new_s;
			thread_args[empty].turn = 0;
			thread_args[empty].thread_index = empty;
			
			//generates a random secret code if non was passed to the server
			// otherwise uses the passed code
			if(secret_code[0]==FLAG){
				generateRandomCode(thread_args[empty].secret_code);
			}
			else{
				memcpy(thread_args[empty].secret_code,secret_code,LENCODE);
			}
			thread_args[empty].secret_code[LENCODE] = '\0';
			//printf("%s\n",thread_args[empty].secret_code);
			
			memcpy(thread_args[empty].ip4,ip4,INET_ADDRSTRLEN);

			get_time(&buffer);
			sprintf(log_line,"<%s>(0.0.0.0)Server secret = %s\n",
				buffer,thread_args[empty].secret_code);
			write_to_log(log_line);

			//creates a thread to serve the game to the client
			int r = pthread_create(&threads[empty],NULL,serve_game,
				(void *)&(thread_args[empty]));
			if (r!=0){
				fprintf(stderr,"Failed to create Thread\n");
				exit(1);
			}
			else thread_count++;
		}	
	}
}
/*
Sends and recieves information from the server to the client.
Delivering the timeless experience that is mastermind...

The game continues until the client either exceeds their turn
limit, 10 turns, or they suceed in guessing the secret code.
The thread records its iteractions with the client in a log, 
taking care to use mutex locks to prevent concurrent access.
Finally after completion of the game the thread, de-increments
the thread count and clears its spot in the array of threads.
*/
void* serve_game(void* arg){
	game *g = arg;
	char code[LENCODE];
	char* results = (char*)malloc(sizeof(char)*LENRESULTS);
	char* buffer = (char*)malloc(sizeof(char)*BUFFERSIZE);
	char* log_line = (char*)malloc(sizeof(char)*BUFFERSIZE);
	int done=0;//0 if NOTDONE, -1 if FAILURE, 1 if SUCCESS

	send(g->socket,WELCOMEMSG,sizeof(WELCOMEMSG),0);

	while (done==0){ 
		if (!recv(g->socket,&code,LENCODE,0)){
			//if error then proceed to wrap up
			break;
		}
		//record client's guess
		get_time(&buffer);
		sprintf(log_line,"<%s>(%s)(Soc_ID %d) client's guess= %s\n",
			buffer,g->ip4,g->socket,code);
		write_to_log(log_line);

		//Processes the client's guess and sends them the results
		done += updateGameState(g,code,results);
		send(g->socket,results,LENRESULTS,0);

		get_time(&buffer);
		sprintf(log_line,"<%s>(0.0.0.0) Server's Response= %s\n",buffer,results);
		write_to_log(log_line);
		
		free(results);
		results = (char*)malloc(sizeof(char)*LENRESULTS);		
	}
	//Close client's socket then perform wrap up of this thread
	close (g->socket);

	pthread_mutex_lock(&lock);

	//frees up space in thread array
	thread_count--;
	threads[g->thread_index]=0;

	if (done>0){
		success_count++;
	}
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}
/*Appends the passed string to the performance log. Locking as required*/
void write_to_log(char* string){
	pthread_mutex_lock(&lock);	
	logfp = fopen(perf_log,"a+");

	fprintf(logfp,"%s", string);
	
	fclose(logfp);
    pthread_mutex_unlock(&lock);
}
/*A signal handler for ctr-C
Cancel the per-client threads and then cancels the main_worker thread.*/
void SIGINT_handler(int sig){
	int i;
	for (i=0;i<MAXTHREADS;i++){
		if (threads[i]!=0)
			pthread_cancel(threads[i]);
	}
	pthread_cancel(main_worker_thread);
}
/*Sets time_string to point to a string representing the current system time.*/
void get_time(char** time_string){	
	time_t curr_time;
	curr_time = time(NULL);
	*time_string = ctime(&curr_time);
	//strips end of line character, credit 
	//http://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
	(*time_string)[strcspn(*time_string, "\n")] = 0;
}
/*
Retrieves selected data from the proc virtual file system
	size: is the Total Virtual Memory Size of the Process.
	rss: is the Resident Set Size of the Process ie. the total number 
		of pages stored in main Memory.
	text: is the Size of the text(program) component of the process.
	data: is the Size of the Stack + the Size of the data(variables)
		of the process.

way to get data from proc modified from code from
http://stackoverflow.com/questions/1558402/memory-usage-of-current-process-in-c
*/
void retrieve_performance_statistics(){
	char* buffer = (char*)malloc(BUFFERSIZE*sizeof(char));
	FILE *stat_f = fopen(stat_path,"r");
	typedef struct {
    	unsigned long size,rss,share,text,lib,data,dt;
	} statm_t;
	statm_t proc_data;

	fscanf(stat_f,"%ld %ld %ld %ld %ld %ld %ld",
    	&proc_data.size,&proc_data.rss,&proc_data.share
    	,&proc_data.text,&proc_data.lib,&proc_data.data,&proc_data.dt);
	
	sprintf(buffer,"VMSize: %ld pages, RSS: %ld pages, Text: %ld pages,"\
		" Data+Stack: %ld pages\n",
		proc_data.size,proc_data.rss,proc_data.text,proc_data.data);
	write_to_log(buffer);
}