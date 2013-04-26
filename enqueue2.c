#include <stdio.h>
#include "retask.h"


int main()
{
    Queue* q = queue_new("example");
    queue_connect(q);
    printf("%d\n", queue_length(q));

    
    char *data = calloc(20, sizeof(char));
    strncpy(data, "\"ac\"", 7);
    Task *t = task_new(data, "0");

    Job *j = queue_enqueue(q, t);

    printf("Got job: %s\n", j->urn);

    job_wait(j, 0);

    printf("Result is %s\n", j->result);

    task_destroy(t);
    job_destroy(j);
    
    queue_destroy(q);
    return 0;
}