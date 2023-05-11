#include "dccthread.h"
#include "dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h>
#include <stdbool.h>

#include <signal.h>
#include <time.h>

typedef struct dccthread
{
    char name[DCCTHREAD_MAX_NAME_SIZE];
    bool yielded;
    bool done;

    dccthread_t *wait;
    ucontext_t context;

} dccthread_t;

dccthread_t manager;
dccthread_t *main_thread;
dccthread_t *current;
struct dlist *ready;

timer_t timerid;

struct sigevent sev;
struct itimerspec its;
struct sigaction sa;

int compare(const void *t1, const void *t2, void *userdata)
{
    dccthread_t *x = (dccthread_t *)t1;
    dccthread_t *y = (dccthread_t *)t2;
    return (x != y);
}

void schedule()
{
    while (!dlist_empty(ready))
    {
        for (int i = 0; i < ready->count; i++)
        {
            current = dlist_get_index(ready, i);
            // [p1, p2, p3]  HEAP
            // p1 -> p4
            if (current->wait != NULL && !current->wait->done)
            {
                dccthread_t *aux = current->wait;

                timer_settime(timerid, 0, &its, NULL);
                swapcontext(&manager.context, &aux->context);

                if (!aux->yielded)
                {
                    dccthread_t *finished = dlist_find_remove(ready, aux, compare, NULL);
                    if (finished != NULL)
                        finished->done = true;
                    current->wait = NULL;
                }
            }
            else if (!current->yielded)
            {
                timer_settime(timerid, 0, &its, NULL);
                swapcontext(&manager.context, &current->context);

                if (!current->yielded)
                {
                    dccthread_t *finished = dlist_find_remove(ready, current, compare, NULL);
                    if (finished != NULL)
                        finished->done = true;
                }
            }
            else
            {
                current->yielded = false;
            }
        }
    }
    // timer_delete(timerid);
}

void dccthread_init(void (*func)(int), int param)
{
    // Definiçoes do timer
    sa.sa_handler = (void *)dccthread_yield;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGRTMIN);
    sa.sa_flags = 0;
    sigaction(SIGRTMIN, &sa, NULL);
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_PROCESS_CPUTIME_ID, &sev, &timerid);

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 10000000; // 10ms
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 10000000; // 10ms

    // criando a fila de prontos
    ready = dlist_create();

    // iniciando manager
    getcontext(&manager.context);
    manager.context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    manager.context.uc_stack.ss_size = THREAD_STACK_SIZE;
    manager.context.uc_link = NULL;
    manager.wait = NULL;
    manager.done = false;

    strcpy(manager.name, "manager");
    makecontext(&manager.context, (void *)schedule, 0, NULL);

    main_thread = dccthread_create("main", func, param);

    schedule();

    exit(1);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *newThread = malloc(sizeof(dccthread_t));

    getcontext(&newThread->context); // inicializando ucontext
    newThread->wait = NULL;
    newThread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE); // informando *ptr
    newThread->context.uc_stack.ss_size = THREAD_STACK_SIZE;       // informando stack size max
    newThread->context.uc_link = &manager.context;                 // informando thread de troca de contexto
    newThread->yielded = false;
    newThread->done = false;
    strcpy(newThread->name, name);

    makecontext(&newThread->context, (void *)func, 1, param); // assinalando uma função para a thread nova

    dlist_push_right(ready, newThread);

    return newThread;
}

void dccthread_yield(void)
{
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
    current->yielded = true;
    swapcontext(&current->context, &manager.context);
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
}

dccthread_t *dccthread_self(void)
{
    return current;
}

const char *dccthread_name(dccthread_t *tid)
{
    return tid->name;
}

void dccthread_exit(void)
{
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
    if (current->wait == NULL)
    {
        current->yielded = false;
        swapcontext(&current->context, &manager.context);
    }
    else
    {
        current->yielded = false;
        current = current->wait;
    }
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
}

void dccthread_wait(dccthread_t *tid)
{
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
    if (!tid->done)
    {
        current->wait = tid;
        swapcontext(&current->context, &tid->context);
    }
    sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL);
}

void dccthread_sleep(struct timespec ts)
{
    
}