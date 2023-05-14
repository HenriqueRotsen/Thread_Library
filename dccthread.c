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

int isIn(dccthread_t *a, struct dlist *lista)
{
    if (lista == NULL)
        return 0;

    for (struct dnode *curr = lista->head; curr != NULL; curr = curr->next)
    {
        dccthread_t *p = curr->data;
        if (p == a)
            return 1;
    };
    return 0;
}

void schedule()
{
    while (!dlist_empty(ready))
    {
        for (int i = 0; i < ready->count; i++)
        {
            current = dlist_get_index(ready, i);

            if (!current->yielded && !isIn(current->wait, ready))
            {
                timer_settime(timerid, 0, &its, NULL);
                swapcontext(&manager.context, &current->context);

                if (!current->yielded && !isIn(current->wait, ready))
                {
                    dlist_find_remove(ready, current, compare, NULL);
                }
            }
            else
            {
                current->yielded = false;
            }
        }
    }
    timer_delete(timerid);
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

    strcpy(manager.name, "manager");
    makecontext(&manager.context, (void *)schedule, 0, NULL);

    main_thread = dccthread_create("main", func, param);

    schedule();

    exit(1);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param)
{
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    dccthread_t *newThread = malloc(sizeof(dccthread_t));

    getcontext(&newThread->context); // inicializando ucontext
    newThread->wait = NULL;
    newThread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE); // informando *ptr
    newThread->context.uc_stack.ss_size = THREAD_STACK_SIZE;       // informando stack size max
    newThread->context.uc_link = &manager.context;                 // informando thread de troca de contexto
    newThread->yielded = false;
    strcpy(newThread->name, name);

    makecontext(&newThread->context, (void *)func, 1, param); // assinalando uma função para a thread nova

    dlist_push_right(ready, newThread);

    sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);

    return newThread;
}

void dccthread_yield(void)
{
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);

    current->yielded = true;
    swapcontext(&current->context, &manager.context);

    sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
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
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    
    current->yielded = false;
    current->wait = NULL;
    swapcontext(&current->context, &manager.context);

    sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
}

void dccthread_wait(dccthread_t *tid)
{
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    if (isIn(tid, ready))
    {
        current->wait = tid;
        current->yielded = true;
        swapcontext(&current->context, &manager.context);
    }
    else //TID ja terminou
    {
        current->yielded = false;
        current->wait = NULL;
    }   
    sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
}

void dccthread_sleep(struct timespec ts)
{
}