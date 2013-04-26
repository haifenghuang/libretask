#include <stdio.h>
#include "retask.h"


int main()
{
    Queue* q = queue_new("example");
    queue_connect(q);
    printf("%d\n", queue_length(q));

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