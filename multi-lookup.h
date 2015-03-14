/*
 * File: multi-lookup.h
 * Author: Jesse Wisniewski
 * Project: CSCI 3753 Programming Assignment 3
 */
/*
#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

#define QUEUEMAXSIZE 50

#define QUEUE_FAILURE -1
#define QUEUE_SUCCESS 0

typedef struct queue_node_s{
    void* payload;
} queue_node;
*/
typedef struct threadstuff{
    FILE* fpoi;
    queue* qpoi;
    int* intpoi;
} test;
