#include "dccthread.h"
#include "dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h>
#include <stdbool.h>

typedef struct dccthread
{
    ucontext_t context;
    char name[DCCTHREAD_MAX_NAME_SIZE];
    bool yelded;

} dccthread_t;

dccthread_t manager;
dccthread_t *main_thread;
dccthread_t *current;
struct dlist *ready;


bool comp(char *name1, char* name2){
    
}


void schedule()
{
    while (!dlist_empty(ready))
    {
        for (int i = 0; i < ready->count; i++)
        {
            current = dlist_get_index(ready, i);
            if (!current->yelded)
            {
                swapcontext(&manager.context, &current->context);
                if(!current->yelded) {
                    dlist_find_remove(ready, current, );
                }
            }
            else
            {
                current->yelded = false;   
            }
            
        }        
    }
}
// [p2(y), p1(y), p3, p4, p5]

void dccthread_init(void (*func)(int), int param)
{
    // criando a fila de prontos
    ready = dlist_create();
    // iniciando manager
    getcontext(&manager.context);
    manager.context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    manager.context.uc_stack.ss_size = THREAD_STACK_SIZE;
    manager.context.uc_link = NULL;
    strcpy(manager.name, "manager");
    makecontext(&manager.context, (void *)schedule, 0, NULL);

    main_thread = dccthread_create("main", func, param);
    schedule();
    exit(1);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *newThread = malloc(sizeof(dccthread_t));

    getcontext(&newThread->context);                               // inicializando ucontext
    newThread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE); // informando *ptr
    newThread->context.uc_stack.ss_size = THREAD_STACK_SIZE;       // informando stack size max
    newThread->context.uc_link = &manager.context;                 // informando thread de troca de contexto
    newThread->yelded = false;
    strcpy(newThread->name, name);

    makecontext(&newThread->context, (void *)func, 1, param); // assinalando uma função para a thread nova
    
    dlist_push_right(ready, newThread);

    return newThread;
}

void dccthread_yield(void)
{
    current->yelded = true;
    swapcontext(&current->context, &manager.context);
}

dccthread_t *dccthread_self(void)
{
    return current;
}

const char *dccthread_name(dccthread_t *tid)
{
    return tid->name;
}