/* myloggerd.c
 * Source file for thread-lab
 * Creates a server to log messages sent from various connections
 * in real time.
 *
 * Student:Paolo Lambre 
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <pthread.h>
#include "message-lib.h"

// forward declarations
int usage( char name[] );
// a function to be executed by each thread
void * recv_log_msgs( void * arg );

// globals
int log_fd; // opened by main() but accessible by each thread
pthread_mutex_t lock;


void * recv_log_msgs( void * arg ){
	
	// loops to receive messages from a connection;
	// when read_msg returns zero, terminate the loop
	// and close the connection
	int connection  = *((int *)arg);
	pthread_mutex_lock(&lock);
	char buffer[1000];
	int chars_read;
	while(chars_read=read_msg(connection,buffer,1000),chars_read>0){
	    printf("%s", buffer);
		write(log_fd,buffer,chars_read);
	}
	pthread_mutex_unlock(&lock);
	close_connection(connection);
	return NULL;
}

int usage( char name[] ){
	printf( "Usage:\n" );
	printf( "\t%s <log-file-name> <UDS path>\n", name );
	return 1;
}

int main( int argc, char * argv[] )
{
	if ( argc != 3 )
		return usage( argv[0] );
		
	// open the log file for appending
	log_fd = open(argv[1],O_CREAT|O_APPEND|O_WRONLY,S_IRUSR | S_IWUSR);
	// permit message connections
	int listener = permit_connections(argv[2]);
	if (listener==-1){
		return -1;
	}
	// loop to accept message connections;
	// as each connection is accepted,
	// launch a new thread that calls
	// recv_log_msgs(), which receives
	// messages and writes them to the log file			
	// when accept_next_connection returns -1, terminate the loop
	int connection;
	pthread_t t[3];
	int i = 0;
	pthread_mutex_init(&lock, NULL);
	while(connection = accept_next_connection(listener),connection!=-1){
		if(connection == -1){
			return -1;
		}
		pthread_create(&t[i],NULL,recv_log_msgs,&connection);
	    pthread_join(t[i], NULL);
	    i = i + 1;
	}
	pthread_mutex_destroy(&lock); 
	// close the listener
	close(listener);
	// close the log file
	close(log_fd);
	return 0;
}
