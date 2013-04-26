#ifndef retask_h_
#define retask_h_
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <hiredis.h>
#include <json/json.h>

typedef struct _Queue Queue;
typedef struct _Task Task;
typedef struct _Job Job;


struct _Queue {
    char* name;
    char* host;
    int port;
    int db;
    char* password;
    redisContext* c;
};

struct _Task {
    char* data;
    const char* urn;
};

struct _Job {
    char* urn;
    char* result;
    Queue* q;
};

Job* job_new(const char *, Queue *);
void job_destroy(Job *);
int job_wait(Job *, int);

Task* task_new(const char *, const char *);
void task_destroy(Task *);

Queue* queue_new(char *, char *, int, int, char *);
void queue_destroy(Queue *);
int queue_connect(Queue *);
long long queue_length(Queue *);
Job* queue_enqueue(Queue *, Task *);
Task* queue_dequeue(Queue *);
Task* queue_wait(Queue *, int);
int queue_send(Task *, char *, Queue *, int);

#endif