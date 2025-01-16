#include <semaphore.h>

#define PHILOSOPHERS 5

#define LEFT(i) (i + PHILOSOPHERS - 1) % PHILOSOPHERS
#define RIGHT(i) (i + 1) % PHILOSOPHERS

enum axis { x, y };
typedef enum state { THINKING, HUNGRY, EATING } State;

typedef struct philosopher {
    unsigned int id;
    State state;
    sem_t chopsticks;
    struct philosopher *l;
    struct philosopher *r;
} Philosopher;
