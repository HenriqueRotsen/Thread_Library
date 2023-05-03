#include "dccthread.h"
#include "dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h>

typedef struct dccthread
{
    ucontext_t context;
    char name[DCCTHREAD_MAX_NAME_SIZE];

} dccthread_t;

dccthread_t manager;
dccthread_t * main;

void schedule(int param){
    while (!dlist_empty(ready)){
        /* code */
    }
    
}

void dccthread_init(void (*func)(int), int param)
{
    // iniciando manager
    getcontext(&manager.context);
    manager.context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    manager.context.uc_stack.ss_size = THREAD_STACK_SIZE;
    strcpy(manager.name, "manager");
    makecontext(&manager.context, (void *)schedule, 0, NULL);

    main = dccthread_create("main", func, param);

    struct dlist * ready = dlist_create();
    dlist_push_right(ready, main);

    setcontext(&main->context);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *newThread = malloc(sizeof(dccthread_t));

// 
    getcontext(&newThread->context);                               // inicializando ucontext
    newThread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE); // informando *ptr
    newThread->context.uc_stack.ss_size = THREAD_STACK_SIZE;       // informando stack size max
    newThread->context.uc_link = &manager.context;                 // informando thread de troca de contexto
    strcpy(newThread->name, name);
// 

    makecontext(&newThread->context, (void *)func, 0, NULL); // assinalando uma função para a thread nova
    return newThread;
}

void dccthread_yield(void)
{
}
