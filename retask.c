#define ENOUGH ((sizeof(char) * sizeof(int) - 1) / 3 + 2)

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
    const char* urn;
    char* result;
    Queue* q;
};


char * get_command_output(const char* cmd){
    FILE *fp;
    char *result = calloc(120, sizeof(char));

    fp = popen(cmd, "r");
    fgets(result, 119, fp);
    fclose(fp);

    result[strlen(result) - 1] = '\0';
    return result;
}


Job* job_new(const char* urn, Queue *q) {
    Job *j = calloc(1, sizeof(Job));
    j->urn = strdup(urn);
    j->q = q;

    return j;
}


void job_destroy(Job *j){
    if (j->urn)
        free((char *)j->urn);
    if (j->result)
        free(j->result);
    free(j);
}

int job_wait(Job *j, int timeout){
    /* 
    We need to do a blocking call on the given urn and queue of the Job.
    */
    redisReply *reply;
    redisReply *reply2;
    char str[ENOUGH];

    sprintf(str, "%d", timeout);

    reply = redisCommand(j->q->c, "brpop %s %s", j->urn, str);
    if (( reply->type == REDIS_REPLY_ERROR )||
            (reply->type == REDIS_REPLY_NIL)) {
        
        freeReplyObject(reply);
        return -1;
    }

    reply2 = reply->element[1];

    j->result = strdup(reply2->str);
    freeReplyObject(reply);

    return 0;

}

Task* task_new(const char *data, const char *uname) {
    Task *t = calloc(1, sizeof(Task));
    t->data = strdup(data);

    if (strncmp(uname, "0", 1) != 0)
        t->urn = uname;
    else
        t->urn = get_command_output("python -c \"import uuid;print uuid.uuid4().urn;\"");

    return t;
}

void task_destroy(Task *t) {
    if (t->data)
        free(t->data);
    if (t->urn)
        free((char *)t->urn);
    if (t)
        free(t);
}

Queue* queue_new(const char *name, const char *host, int port, int db, const char *password){
    Queue* q = calloc(1, sizeof(Queue));
    
    /* Create the name of the queue */
    char *path = calloc(100, sizeof(char));
    strncpy(path, "retaskqueue-", sizeof("retaskqueue-"));
    strncat(path, name, sizeof(name));

    q->name = path;
    q->host = strdup(host);
    q->port = port;
    q->db = db;
    if (password)
        q->password = strdup(password);

    return q;
}

void queue_destroy(Queue* self){
    if (self->name)
        free(self->name);
    if (self->host)
        free(self->host);
    if (self->password)
        free(self->password);
    if (self->c)
        redisFree(self->c);
    if (self)
        free(self);
}

int queue_connect(Queue* self) {
    redisReply *reply;

    self->c = redisConnect(self->host, self->port);
    if (self->c->err)
        return -1;
    // if (self->password)
    // {
    //     reply = redisCommand(self->c, "auth %s", self->name);
    //     if ( reply->type == REDIS_REPLY_ERROR )
    //     {
    //         printf( "Error: %s\n", reply->str );
    //         freeReplyObject(reply);
    //         return -1;
    //     }
    // }

    // if (self->db != 0)
    // {
    //     reply = redisCommand(self->c, "select %d", self->db);
    //     if ( reply->type == REDIS_REPLY_ERROR )
    //     {
    //         printf( "Error: %s\n", reply->str );
    //         freeReplyObject(reply);
    //         return -1;
    //     }
    // }

    // freeReplyObject(reply);
    return 0;
}

long long queue_length(Queue *self){
    redisReply *reply;
    long long ret;

    /* Find the length of the given queue*/
    reply = redisCommand(self->c, "llen %s", self->name);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
        return -1;
    }

    ret = reply->integer;
    freeReplyObject(reply);
    return ret;

}

Job* queue_enqueue(Queue* self, Task* task){
    json_object *root = json_object_new_object();

    json_object_object_add(root, "_data", json_object_new_string(task->data));
    json_object_object_add(root, "urn", json_object_new_string(task->urn));

    const char *tmp = json_object_to_json_string(root);
    
    redisReply *reply;
    reply = redisCommand(self->c, "lpush %s %s", self->name, tmp);


    freeReplyObject(reply);

    /*Now create a Job object and return it*/
    Job *job = job_new(task->urn, self);

    /* Free JSON object */
    json_object_put(root);

    return job;
}

Task* queue_dequeue(Queue* self){
    if (!self->c)
        return NULL;

    redisReply *reply;
    /*
    char *key;
    struct json_object *val;
    */
    const char *data, *data2;
    const char *urn, *urn2;
    
    /* First get the reply */
    reply = redisCommand(self->c, "rpop %s", self->name);
    if (( reply->type == REDIS_REPLY_ERROR )||
            (reply->type == REDIS_REPLY_NIL)) {
        
        freeReplyObject(reply);
        return NULL;
    }
    
    /* Now get the data and urn to create Task object */
    json_object * jobj = json_tokener_parse(reply->str);
    json_object_object_foreach(jobj, key, val){
        if (strncmp(key, "_data", 5) == 0)
            data = json_object_get_string(val);
        if (strncmp(key, "urn", 3) == 0)
            urn = json_object_get_string(val);
    }

    data2 = data;
    urn2 = strdup(urn);
    Task *task = task_new(data2, urn2);
    


    /* First free all JSON objects*/
    freeReplyObject(reply);
    json_object_put(jobj);
 
    return task;
}


Task* queue_wait(Queue* self, int timeout){
    if (!self->c)
        return NULL;

    redisReply *reply;
    redisReply *reply2;
    char str[ENOUGH];

    sprintf(str, "%d", timeout);
    /*
    char *key;
    struct json_object *val;
    */
    const char *data, *data2;
    const char *urn, *urn2;
    
    /* First get the reply */
    reply = redisCommand(self->c, "brpop %s %s", self->name, str);
    if (( reply->type == REDIS_REPLY_ERROR )||
            (reply->type == REDIS_REPLY_NIL)) {
        
        freeReplyObject(reply);
        return NULL;
    }

    reply2 = reply->element[1];

    
    /* Now get the data and urn to create Task object */
    json_object * jobj = json_tokener_parse(reply2->str);
    json_object_object_foreach(jobj, key, val){
        if (strncmp(key, "_data", 5) == 0)
            data = json_object_get_string(val);
        if (strncmp(key, "urn", 3) == 0)
            urn = json_object_get_string(val);
    }

    data2 = data;
    urn2 = strdup(urn);
    Task *task = task_new(data2, urn2);
    


    /* First free all JSON objects*/
    freeReplyObject(reply);
    json_object_put(jobj);
 
    return task;
}


int queue_send(Task *t, char *result, Queue* self, int timeout) {
    redisReply *reply;
    char str[ENOUGH];
    if (!self->c)
        return -1;


    sprintf(str, "%d", timeout);
    /* Let us just send the result*/
    reply = redisCommand(self->c, "lpush %s %s", t->urn, result);
    reply = redisCommand(self->c, "expire %s %d", t->urn, str);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
        return -2;
    }

    freeReplyObject(reply);
    
    return 0;
}