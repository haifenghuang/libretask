#include <stdio.h>
#include "retask.h"


int main()
{
    Queue* q = queue_new("example");
    queue_connect(q);
    printf("%d\n", queue_length(q));

    /*
    char *data = calloc(20, sizeof(char));
    strncpy(data, "\"ac\"", 7);
    Task *t = task_new(data, "0");

    Job *j = queue_enqueue(q, t);

    printf("Got job: %s\n", j->urn);

    task_destroy(t);
    job_destroy(j);
    */

    Task* t = queue_wait(q, 0);


    printf("Received: %s URN: %s \n", t->data, t->urn);


    task_destroy(t);    
    queue_destroy(q);
    return 0;
}