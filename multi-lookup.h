/*
 * File: multi-lookup.h
 * Author: Jesse Wisniewski
 * Project: CSCI 3753 Programming Assignment 3
 */
typedef struct threadstuff{
    FILE* fpoi;
    queue* qpoi;
    int* intpoi;
    pthread_mutex_t* mutqpoi;
    pthread_mutex_t* mutoutfpoi;
    pthread_cond_t* condfullpoi;
    pthread_cond_t* condemptpoi;
} threadvars;
