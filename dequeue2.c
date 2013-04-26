#include <stdio.h>
#include "retask.h"


int main()
{
    Queue* q = queue_new("example", "localhost", 6379, 0, NULL);
    queue_connect(q);
    printf("Current Queue Length: %lld\n", queue_length(q));

    Task* t = queue_dequeue(q);
    if (!t) {
        printf("No job in Queue.\n");
        queue_destroy(q);
        return 0;
    }


    char *data = calloc(20, sizeof(char));
    strncpy(data, "42", 2);

    printf("Received: %s URN: %s \n", t->data, t->urn);

    queue_send(t, data, q, 60);

    task_destroy(t);    
    queue_destroy(q);
    return 0;
}