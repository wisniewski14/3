/*
 * File: multi-lookup.c
 * Author: Jesse Wisniewski
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2015/03/13
 * Modify Date: 2015/03/16
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "queue.h"
#include "multi-lookup.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define Q_SIZE 100
#define NUM_THREADS 2

void* Requester_function(void* vpoi){
	char* hostnames[Q_SIZE];
	int i;
	struct threadstuff* stuf = vpoi;
	/* Setup hostnames as char* array from
	 * 0 to Q_SIZE-1 */
	for(i=0; i<Q_SIZE; i++){
	    hostnames[i] = malloc(sizeof(*(hostnames[i])));
	}
	i=0;
	/* Read File and Process*/
	while(fscanf(stuf->fpoi, INPUTFS, hostnames[i]) > 0){

	    /* Wait for queue to have room */
	    while(queue_is_full(stuf->qpoi)){
		//wait
	    }
	    /* Write to queue */
	    pthread_mutex_lock(stuf->mutqpoi);
	    if(queue_push(stuf->qpoi, hostnames[i]) == QUEUE_FAILURE){
              fprintf(stderr,
                    "error: queue_push failed!\n"
                    "Hostname: %s\n",
                    hostnames[i]);
            }else{
              fprintf(stderr, "\nPushed hostname: %s\n", hostnames[i]);
	    }
	    pthread_mutex_unlock(stuf->mutqpoi);
            i=i+1;
	    if(i==Q_SIZE){
		i=0;
	    }
	}
	return NULL;
} 

void* Resolver_function(void* vpoi){
    char* hname;
    char firstipstr[INET6_ADDRSTRLEN];
    struct threadstuff* stuf = vpoi;
    while(!*(stuf->intpoi)){
        while(queue_is_empty(stuf->qpoi)){
            //wait
        }
	pthread_mutex_lock(stuf->mutqpoi);
	if((hname = queue_pop(stuf->qpoi)) == NULL){
          fprintf(stderr,
                  "error: queue_pop failed!\n");
        }else{
             fprintf(stderr,"\nPopped: %s", hname);
	}
	pthread_mutex_unlock(stuf->mutqpoi);
	/* Pop successful */
	/* Lookup hostname and get IP string */
	if(dnslookup(hname, firstipstr, sizeof(firstipstr))
	   == UTIL_FAILURE){
	    fprintf(stderr, "dnslookup error: %s\n", hname);
	    strncpy(firstipstr, "", sizeof(firstipstr));
	}else{
	    /* Lookup, successful, Write to Output File */
	    fprintf(stuf->fpoi, "%s,%s\n", hname, firstipstr);
	}
    }
	return NULL;
}




int main(int argc, char* argv[]){

    /* Local Vars */
    queue theq;
    const int qSize = Q_SIZE;
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    char errorstr[SBUFSIZE];
    int i;
    int rc;
    int numstructs=argc-1;
    numstructs=numstructs;
    pthread_t threads[NUM_THREADS];

    pthread_mutex_t mutQ;
    pthread_mutex_t mutF;
    pthread_cond_t condE;
    pthread_cond_t condF;

    /* Need to malloc these? XXXXXXX */ 
    struct threadstuff stuffs[2];     
    int finish[1];

    /* Initialize mutexes and condition variables*/
    pthread_mutex_init(&mutQ,NULL);
    pthread_mutex_init(&mutF,NULL);
    pthread_cond_init(&condE,NULL);
    pthread_cond_init(&condF,NULL);

    /* Initialize Queue */
    if(queue_init(&theq, qSize) == QUEUE_FAILURE){
        fprintf(stderr,
                "error: queue_init failed!\n");
    }

    /* Check Arguments */
    if(argc < MINARGS){
	fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
	fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
    }

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
	perror("Error Opening Output File");
	return EXIT_FAILURE;
    }


    /* Struct for resolver threads */
    stuffs[0].fpoi = outputfp;
    stuffs[0].qpoi = &theq;
    stuffs[0].intpoi = finish;
    stuffs[0].mutqpoi = &mutQ;
    stuffs[0].mutoutfpoi = &mutF;

    finish[0]=0;

    /* Spawn resolver thread */
    printf("In main: creating resolver thread\n");
    rc = pthread_create(&(threads[0]), NULL, Resolver_function, &stuffs[0]);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(EXIT_FAILURE);
    }

    /* Loop Through Input Files */
    for(i=1; i<(argc-1); i++){
	
	/* Open Input File */
	inputfp = fopen(argv[i], "r");
	if(!inputfp){
	    sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
	    perror(errorstr);
	    break;
	}	
	/* Structs for requester threads */
	stuffs[1].fpoi = inputfp;
	stuffs[1].qpoi = &theq;
	stuffs[1].intpoi = &finish[0];
        stuffs[1].mutqpoi = &mutQ;
        stuffs[1].mutoutfpoi = &mutF;

        /* Spawn requester thread */
        printf("In main: creating requester thread\n");
        rc = pthread_create(&(threads[1]), NULL, Requester_function, &stuffs[1]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
	(void) pthread_join(threads[1], NULL);
	// function calls - non-threaded 
	//Requester_function(&stuffs[1]);
	//Resolver_function(&stuffs[0]);
	

	/* Close Input File */
	fclose(inputfp);
    }

    while(!queue_is_empty(&theq)){
       //wait
            //printf("Waiting on resolver thread\n");
    }

    finish[0]=1;
    // wait for resolver thread to close
    (void) pthread_join(threads[0], NULL);

    /* Close Output File */
    fclose(outputfp);


    return EXIT_SUCCESS;
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void* Requester_nonthread(FILE* inf, queue* qpoint){
	char* hostnames[Q_SIZE];
	int i;
	// Setup hostnames as char* array from
	// 0 to Q_SIZE-1 
	for(i=0; i<Q_SIZE; i++){
	    hostnames[i] = malloc(sizeof(*(hostnames[i])));
	}
	i=0;
	// Read File and Process
	while(fscanf(inf, INPUTFS, hostnames[i]) > 0){
	    // Write to queue 
	    if(queue_push(qpoint, hostnames[i]) == QUEUE_FAILURE){
              fprintf(stderr,
                    "error: queue_push failed!\n"
                    "Hostname: %s\n",
                    hostnames[i]);
            }else{
              //fprintf(stderr, "\nPushed hostname: %s\n", hostname);
	    }
            i=(i+1)%(Q_SIZE-1);
	}
	return NULL;
} 
void* Resolver_nonthread(FILE* outputfile, queue* qpoint){
    char* hname;
    char firstipstr[INET6_ADDRSTRLEN];

    while(!queue_is_empty(qpoint)){
	if((hname = queue_pop(qpoint)) == NULL){
          fprintf(stderr,
                  "error: queue_pop failed!\n");
        }else{
          fprintf(stderr,"\nPopped: %s", hname);
        }
	    // Lookup hostname and get IP string 
	    if(dnslookup(hname, firstipstr, sizeof(firstipstr))
	       == UTIL_FAILURE){
		fprintf(stderr, "dnslookup error: %s\n", hname);
		strncpy(firstipstr, "", sizeof(firstipstr));
	    }
	    // Write to Output File 
	    fprintf(outputfile, "%s,%s\n", hname, firstipstr);
    }
	return NULL;
}
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/
