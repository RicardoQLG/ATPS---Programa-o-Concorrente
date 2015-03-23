#include <stdio.h>     // Funções de input e output

#include <pthread.h>   // Thread que é a base da aplicação
#include <semaphore.h> // Funções de semáforo em C

#include <fcntl.h>     // Necessário para O_CREAT

#ifdef __unix__
  #include <unistd.h>  // Funciona para Linux
#elif defined _WIN32
  #include <windows.h> // Funciona para windows
  #define sleep(x) Sleep(1000 * x) // Adapta o sleep
#endif

#define READ_WRITE "w+" // Modo de gravação do CSV
#define EXPORT_AS_CSV TRUE // Deve exportar aquivo CSV
#define SEPARATOR ";"   // Separador do arquivo
#define DEBUG          // Modo de debug (commente para o ver em produção)

#ifndef DEBUG
  #define POSITIONS 5000        // Quantidade de elementos no buffer
  #define USLEEP_TIME 10        // Tempo de espera do processo
#else
  #define POSITIONS 20          // Quantidade de elementos no buffer
  #define USLEEP_TIME 1         // Tempo de espera do processo
#endif // DEBUG

#define CONCURRENT_CONSUMERS 5 // Numnero de threads

// Armazena o buffer que será processado pela thread
struct Buffer {
    int values[POSITIONS]; // Valores armazenados
    int current_position;  // Posição atual de trabalho
} buffer;

// Armazena as threads criadas
pthread_t concurrent_consumers[CONCURRENT_CONSUMERS];

// Armazena o numero de threads em processamento
int num_threads;
// Guarda textos do CSV
char stdio[1024];
// Aponta para arquivo CSV
FILE *csv_handler;
// Semáforo usado para controlar threads
sem_t semaphore;

/**
 * Consumidor
 */
void *consumer (void *arg)
{
  // Guarda o id da thread atual
  pthread_t self = pthread_self();
  // Exibe na tela a thread sendo iniciada
  printf("Criando thread %d\n", self);

   // Enquando houverem trabalhos a serem executados
  while (buffer.current_position < POSITIONS)
  {
    // armazena o buffer de trabalho atual na váriavel auxiliar
    int n = buffer.current_position++;
    // Aguarda o semáforo liberar a execução do trabalho
    sem_wait(&semaphore);

    // Guarda o valor a ser processado
    int value = buffer.values[n];
    // timestamp do inicio da tarefa
    time_t start_time = time(NULL);



    /* Tarefa */
    sleep(USLEEP_TIME); // Aguarde por um tempo
    /* Tarefa */


    // timestamp do fim da tarefa
    time_t end_time = time(NULL);

    printf("Pedido: %d\n", n);
    printf("Thread id:%lu\n", self);
    printf("Inicio:%d(s), ", start_time);
    printf("Fim:%d(s), ", end_time);
    printf("Duracao:%d(s)\n\n", end_time - start_time);

    if (EXPORT_AS_CSV) // Se for necessário exportar um CSV
    {
      // limpa os dados da variável auxiliar
      memset(stdio,0,strlen(stdio));
      // Grava os novos dados da variável
      sprintf(stdio,
          "%d%s%lu%lu%s%d%s%d%s%d\n",
          n,
          SEPARATOR,
          self,
          SEPARATOR,
          start_time,
          SEPARATOR,
          end_time,
          SEPARATOR,
          end_time - start_time
        );
      // Grava no arquivo CSV
      fwrite (stdio, 1, sizeof(stdio), csv_handler);
    }

    // Informa ao semáforo que o trabalho foi finalizado
    sem_post(&semaphore);
  }
  
  num_threads--;
}

int main (int argc, char const *argv[])
{
  int i; // Declaração da variável auxiliar
  sem_init(&semaphore, 0, CONCURRENT_CONSUMERS); // inicia o semáforo

  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    // Zera o tamanho do buffer de armazenamento de textos
    memset(stdio,0,strlen(stdio));

    // Abre o arquivo csv para escrita
    csv_handler = fopen ( "data.csv", READ_WRITE ); 
    // Guarda o header do CSV na variável
    sprintf(stdio,
        "Pedido:%sThread id:%sInicio: (s)%s Fim: (s)%s Duracao: (s)\n",
        SEPARATOR,
        SEPARATOR,
        SEPARATOR,
        SEPARATOR);

    // Escreve o header no CSV
    fwrite (stdio, 1, sizeof(stdio), csv_handler);
  }

  // Zera a posição atual o buffer
  buffer.current_position = 0;

  // Para cada thread de consumidor
  for (i = 0; i < CONCURRENT_CONSUMERS; i++)
  {
    // Cria a nova thread
    pthread_create(&(concurrent_consumers[i]), NULL, &consumer, (void *)i);
    // incrementa para cada thread iniciada
    num_threads++;
  }

  // Permanece com o processo em execução enquanto houverem thread abertas
  while (num_threads);

  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    // Fecha io arquivo CSV
    fclose (csv_handler);
  }

  return 0;
}
