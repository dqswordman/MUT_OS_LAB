#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h> // for usleep()
#include <stdlib.h> // for exit() and random generator
#include <wait.h>   // for wait()
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include "/home/miic1225/Downloads/lab/conio.h"

#define CHAIRNUM 5

sem_t chopstick[CHAIRNUM];
int timeUsed[CHAIRNUM];

void randomDelay(void);
void *philosopher(void *who);

int main(){
    pthread_t  tid[CHAIRNUM];   // Child Process ID
    pthread_attr_t attr[CHAIRNUM];
    int param[CHAIRNUM];
    int i;

    clrscr();
    // Initialize semaphores
    for(i = 0; i < CHAIRNUM; i++){
        param[i] = i;
        timeUsed[i] = 0;
        sem_init(&chopstick[i], 0, 1); // Initialize the semaphores for the chopsticks
    }

    for(i = 0; i < CHAIRNUM; i++)
        pthread_attr_init(&attr[i]);

    for(i = 0; i < CHAIRNUM; i++)
        pthread_create(&tid[i], &attr[i], philosopher, (void *)&param[i]);

    for(i = 0; i < CHAIRNUM; i++)
        pthread_join(tid[i], NULL);

    for(i = 0; i < CHAIRNUM; i++)
        sem_destroy(&chopstick[i]);

    return 0;
}

void *philosopher(void *who){
    int no = (int)*((int *)who);
    int i = 0;

    for(i = 0; i < 10; i++){
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                                    \n", 'A' + no); fflush(stdout);
        randomDelay();

        // 强制哲学家同时拿起两只筷子
        if (no % 2 == 0) {
            sem_wait(&chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
            sem_wait(&chopstick[no]);  // 拿左边的筷子
        } else {
            sem_wait(&chopstick[no]);  // 拿左边的筷子
            sem_wait(&chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
        }

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c has taken both chopsticks and is eating...        \n", 'A' + no); fflush(stdout);

        // Critical Section
        randomDelay();
        timeUsed[no]++;
        timeUsed[(no + 1) % CHAIRNUM]++;
        gotoxy(no * 10 + 1, CHAIRNUM * 4 + 1);
        printf("CH[%d]=%d ", no, timeUsed[no]);
        gotoxy(((no + 1) % CHAIRNUM) * 10 + 1, CHAIRNUM * 4 + 1);
        printf("CH[%d]=%d ", (no + 1) % CHAIRNUM, timeUsed[(no + 1) % CHAIRNUM]);
        gotoxy(no * 10 + 1, CHAIRNUM * 4 + 2);
        printf("  [%c]=%d ", 'A' + no, i);

        gotoxy(1, no * 4 + 1);
        printf("                                                                 \n");
        gotoxy(1, no * 4 + 2);
        printf("                                                                 \n");
        randomDelay();

        // 释放两只筷子
        sem_post(&chopstick[no]);
        sem_post(&chopstick[(no + 1) % CHAIRNUM]);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c drops both chopsticks...                             \n", 'A' + no); fflush(stdout);

        // Remaining Section
        randomDelay();
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is chewing food...                             \n", 'A' + no); fflush(stdout);
        randomDelay();
    }

    gotoxy(1, no * 4 + 1);
    printf("Mr. %c is full...                                         \n", 'A' + no); fflush(stdout);
    pthread_exit(0);
}

void randomDelay(void){
// This function provides a delay which slows the process down so we can see what happens
   srand(time(NULL));
   int stime = ((rand()%2000)+100)*1000;
   usleep(stime);
}
