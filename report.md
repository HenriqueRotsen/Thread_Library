# PAGINADOR DE MEMÓRIA - RELATÓRIO

1. Termo de compromisso

  Os membros do grupo afirmam que todo o código desenvolvido para este
  trabalho é de autoria própria.  Exceto pelo material listado no item
  3 deste relatório, os membros do grupo afirmam não ter copiado
  material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço
  
  Preencha as linhas abaixo com o nome e o e-mail dos integrantes do
  grupo.  Substitua marcadores `XX` pela contribuição de cada membro
  do grupo no desenvolvimento do trabalho (os valores devem somar
  100%).

* Henrique Rotsen Santos Ferreira <henriqueferreira@dcc.ufmg.br> 50%
* Gabriel Castelo Branco Rocha Alencar Pinto <gabriel-castelo@ufmg.br> 50%

3. Referências bibliográficas

4. Estruturas de dados

  4.1.  Descreva e justifique as estruturas de dados utilizadas para
  gerência das threads de espaço do usuário (partes 1, 2 e 5).
    Para o gerenciamento das thhreads em espaço de usuário, foram utilizadas as seguintees estruturas:

      **dccthread_t**: tipo de dados que define uma thread. Todas as threads do programa são do tipo dccthread_t. Um objeto do tipo dccthread_t posui os seguintes atributos:

      * char name: Nome da thread a ser executada

      * bool yielded: Indicador se a thread corrente teve sua execução suspensa através do método _yield

      * dccthread_t *wait: Ponteiro para uma thread que a cuja thread corrente espera a finalização da execução

      * ucontext_t context: Contexto de execução da thread corrente.

      Em cima dessas estruturas de dados, são definidas funções

      **dccthread_init** : 

  4.2.  Descreva o mecanismo utilizado para sincronizar chamadas de
      dccthread_yield e disparos do temporizador (parte 4).
