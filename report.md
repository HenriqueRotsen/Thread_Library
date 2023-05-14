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

    **dccthread_t**: tipo de dados que define uma thread. Todas as threads do programa são do tipo dccthread_t. Um objeto do tipo dccthread_t possui os seguintes atributos:

    * char name: Nome da thread a ser executada

    * bool yielded: Indicador se a thread corrente teve sua execução suspensa através do método `dccthread_yield`

    * dccthread_t *wait: Ponteiro para uma thread que a cuja thread corrente espera a finalização da execução

    * ucontext_t context: Contexto de execução da thread corrente.
 
    Além disso, é definida uma lista do tipo `dlist`, uma lista que contém as threads que ainda precisam ser executadas. Essa lista será gerenciada pela função `schedule()`, que funcionará como o escalonador. Ela irá, de forma rotativa, distribuir o tempo de processamento entre as funções que ainda não finalizaram sua execução.


    Em cima dessas estruturas de dados, são definidas funções para manipular as threads em espaço de usuário. As funções são as seguintes:

      **dccthread_init** : Função que inicializa a thread principal, chamada 'main', além de criar o manager, que é responsável pelo gerenciamento da execução. Na thread manager corre o scheduler, que gerencia a execução das demais threads. Além disso, nesta função são definidos detalhes do timer, que cronometra o tempo de CPU de cada uma das threads e a fila de pronto.

      **dccthread_create** : Cria uma nova thread para executar uma determinada função, que é passada como parâmetro pelo usuário. A função é necessária para alocar a memória necessária pela thread e, sendo bem sucedida em inicializar a nova thread, retorna um ponteiro para a mesma.

      **dccthread_yield** : Paraliza a execução de uma thread. Para isso, marca o atributo `yielded` como true, e troca o contexto de execução para o contexto da thread manager, que executa o schedueler, e, portanto, irá retomar a execução da próxima thread não yieldada da lista de pronto.

      **dccthread_self** : Retorna um ponteiro para a thread em execução.

      **dccthread_name** : Retorna uma lista de caracteres com o nome da thread passada por parâmetro.

      **dccthread_wait** : Indica que uma thread deve esperar outra terminar de executar antes de seguir sua execução padrão.

      **dccthread_exit** : Encerra a execução da thread atual, retirando-a da fila de pronto.

    4.2.  Descreva o mecanismo utilizado para sincronizar chamadas de
        dccthread_yield e disparos do temporizador (parte 4).
