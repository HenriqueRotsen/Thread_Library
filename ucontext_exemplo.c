#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include "dccthread.h"
#include "dlist.h"

typedef struct dccthread
{
    char name[DCCTHREAD_MAX_NAME_SIZE];
    ucontext_t context;
    int yield;
    dccthread_t *waitingFor;
} dccthread_t;

dccthread_t *managerThread;

struct dlist *fila_de_prontos;

void manager_func(int param)
{

    while (!dlist_empty(fila_de_prontos))
    {
        printf("Gerenciando fila de prontos...\n");
        printf("parametro %d\n", param);

        dccthread_t *proximo_a_execultar = dlist_pop_left(fila_de_prontos);

        // salva o contexto do manager (PC, registradores ...) e carrega/executa do proximo
        swapcontext(&managerThread->context, &proximo_a_execultar->context);

        // quando proximo acabar ira voltar para o manager, por causo do uc_link
        // e volta a partir daqui!
    }
}
void func1(int param)
{
    printf("Sou a func 1 com parametro: %d\n", param);
}
void func2(int param)
{
    printf("Sou a func 2 com parametro: %d\n", param);
}

int main(int argc, char const *argv[])
{
    managerThread = (dccthread_t *)malloc(sizeof(dccthread_t));
    // nomeando a thread
    strcpy(managerThread->name, "manager");
    // INICIA as estuturas para o user context (ucontext)
    getcontext(&managerThread->context);
    // define qual o stack pointer ( endereço de mem para a pilha )
    managerThread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    // define tamanho da pilha
    managerThread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    // cria a função do contexto, ou seja, define seu comportamento
    makecontext(&managerThread->context, (void *)manager_func, 1, 13);

    dccthread_t *func1Thread = (dccthread_t *)malloc(sizeof(dccthread_t));
    // nomeando a thread
    strcpy(func1Thread->name, "Função 1");
    // INICIA as estuturas para o user context (ucontext)
    getcontext(&func1Thread->context);
    // define qual o stack pointer ( endereço de mem para a pilha )
    func1Thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    // define tamanho da pilha
    func1Thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    // quando func1 acabar irá chaamr uc_link, ou seja, o manager
    func1Thread->context.uc_link = &managerThread->context;
    // cria a função do contexto, ou seja, define seu comportamento
    makecontext(&func1Thread->context, (void *)func1, 1, 9);

    dccthread_t *func2Thread = (dccthread_t *)malloc(sizeof(dccthread_t));
    // nomeando a thread
    strcpy(func2Thread->name, "Função 2");
    // INICIA as estuturas para o user context (ucontext)
    getcontext(&func2Thread->context);
    // define qual o stack pointer ( endereço de mem para a pilha )
    func2Thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    // define tamanho da pilha
    func2Thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    // quando func2 acabar irá chaamr uc_link, ou seja, o manager
    func2Thread->context.uc_link = &managerThread->context;
    // cria a função do contexto, ou seja, define seu comportamento
    makecontext(&func2Thread->context, (void *)func2, 1, 3);

    fila_de_prontos = dlist_create();
    dlist_push_right(fila_de_prontos, func1Thread);
    dlist_push_right(fila_de_prontos, func2Thread);

    // seta o contexto do usuário para o manager que irá executar manager_func
    setcontext(&managerThread->context);

    return 0;
}
