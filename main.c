#include <stdio.h>     // Funções de input e output

#include <pthread.h>   // Thread que é a base da aplicação

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
//#define DEBUG          // Modo de debug (commente para o ver em produção)

#ifndef DEBUG
  #define POSITIONS 5000        // Quantidade de elementos no buffer
  #define USLEEP_TIME 10        // Tempo de espera do processo
  #define TOTAL_CICLES 10
#else
  #define POSITIONS 20        // Quantidade de elementos no buffer
  #define USLEEP_TIME .001      // Tempo de espera do processo
  #define TOTAL_CICLES 1
#endif // DEBUG

#define MAX_THREADS_NUMBER 1000 // Numnero máximo de threads
#define QUEUE_LENGTH 50000

// Armazena o buffer que será processado pela thread
struct Buffer {
    int values[POSITIONS]; // Valores armazenados
    int current_position;  // Posição atual de trabalho
} buffer;

// Armazena as threads criadas
pthread_t concurrent_consumers[MAX_THREADS_NUMBER];

// Armazena o numero de threads em processamento
int num_threads;
// Guarda textos do CSV
char stdio[256];
char queue[QUEUE_LENGTH];

// Aponta para arquivo CSV
FILE *csv_handler;

void insertCSVData ()
{
    printf(queue);
    fwrite ("\n", 1, sizeof("\n"), csv_handler);
    int i;
    fwrite (queue, 1, sizeof(queue), csv_handler);
    fwrite ("\n", 1, sizeof("\n"), csv_handler);
    memset(stdio,0,strlen(stdio));
}

void queueCSVData (char *line)
{
    char *tmp;
    if (strlen(queue) + strlen(line) > QUEUE_LENGTH) {
      printf("%lu",strlen(queue) + strlen(line));
      exit(1);
    }
    strcat(queue, line);
    free(tmp);
    free(line);
}

/**
 * Consumidor
 */
void consumer (pthread_t self)
{
  // armazena o buffer de trabalho atual na váriavel auxiliar
  int n = buffer.current_position++;

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
}

// Cria a thread
void *thread(void *arg)
{
  // Guarda o id da thread atual
  pthread_t self = pthread_self();
  // Exibe na tela a thread sendo iniciada
  printf("Criando thread %d\n", self);

  time_t ini = time(NULL);

  // Enquando houverem trabalhos a serem executados
  while (buffer.current_position < POSITIONS)
  {
    consumer(self);
  }
  time_t fim = time(NULL);

  sprintf(stdio,
          "Thread%s%lu%lu%s%d%s%d%s%d\n",
          SEPARATOR,
          self,
          SEPARATOR,
          ini,
          SEPARATOR,
          fim,
          SEPARATOR,
          fim - ini
        );
  // Grava no arquivo CSV
  queueCSVData(stdio);
  memset(stdio, 0, strlen(stdio));

  num_threads--;

}

int execute (int threads_number)
{
  int i; // Declaração da variável auxiliar

  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    // Zera o tamanho do buffer de armazenamento de textos
    memset(stdio,0,strlen(stdio));

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
  time_t start_time = time(NULL);

  // Para cada thread de consumidor
  for (i = 0; i < threads_number; i++)
  {
    // Cria a nova thread
    pthread_create(&(concurrent_consumers[i]), NULL, &thread, (void *)i);
    // incrementa para cada thread iniciada
    num_threads++;
  }

  // Permanece com o processo em execução enquanto houverem thread abertas
  while (num_threads);
  time_t end_time = time(NULL);

  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    insertCSVData();
    memset (stdio, 0, strlen(stdio));
    sprintf(stdio,
        "Tempo total de processamento:%s%s%d%s%d%s%d\n",
        SEPARATOR,
        SEPARATOR,
        start_time,
        SEPARATOR,
        end_time,
        SEPARATOR,
        end_time - start_time
      );

    // Grava no arquivo CSV
    fwrite (stdio, 1, sizeof(stdio), csv_handler);

    fwrite ("\n", 1, sizeof("\n"), csv_handler);
  }
  return 0;
}

int main (int argc, char const *argv[])
{
  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    // Abre o arquivo csv para escrita
    csv_handler = fopen ( "data.csv", READ_WRITE );
  }

  int i;
  int thread[6] = {1,10,50,100,500,1000};
  int current_num_threads = 0;

  for (current_num_threads = 0; current_num_threads < 6; current_num_threads++)
  {
    i = TOTAL_CICLES;
    while (i--)
    {
      memset(queue,0,strlen(queue));
      execute(thread[current_num_threads]);
    }
  }

  if (EXPORT_AS_CSV) // Se pediu exportação em CSV
  {
    // Fecha io arquivo CSV
    fclose (csv_handler);
  }

  return 0;
}
