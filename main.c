#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <pthread.h>
#include <semaphore.h>

#define DEBUG

#ifndef DEBUG

#define POSITIONS 5000
#define USLEEP_TIME 10000000

#else

#define POSITIONS 10
#define USLEEP_TIME 500000

#endif // DEBUG

// Armazena dados de tempo para a funções time
struct Timeval {
    time_t seconds;  /* seconds */
    long   microseconds; /* microseconds */
} timeval;

// Armazena o buffer que será processado pela thread
struct Buffer {
    int values[POSITIONS];
    int current_position;
} buffer;

int main (int argc, char const *argv[])
{
    return 0;
}
