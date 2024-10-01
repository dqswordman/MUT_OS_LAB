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

#define CHAIRNUM 5  // 保留5把椅子
#define PHILO_NUM 4 // 只允许4个哲学家参与进餐

sem_t chopstick[CHAIRNUM];  // 筷子的信号量
int timeUsed[CHAIRNUM];

void randomDelay(void);
void* philosopher(void* who);

int main() {
    pthread_t tid[PHILO_NUM];   // 哲学家的线程ID
    pthread_attr_t attr[PHILO_NUM];
    int param[PHILO_NUM];
    int i;

    clrscr();

    // 初始化信号量，仍然是5根筷子
    for (i = 0; i < CHAIRNUM; i++) {
        param[i] = i;
        timeUsed[i] = 0;
        sem_init(&chopstick[i], 0, 1);  // 初始化每根筷子的信号量
    }

    // 创建4个哲学家线程
    for (i = 0; i < PHILO_NUM; i++)
        pthread_attr_init(&attr[i]);

    for (i = 0; i < PHILO_NUM; i++)
        pthread_create(&tid[i], &attr[i], philosopher, (void*)&param[i]);

    // 等待所有哲学家线程结束
    for (i = 0; i < PHILO_NUM; i++)
        pthread_join(tid[i], NULL);

    // 释放筷子的信号量
    for (i = 0; i < CHAIRNUM; i++)
        sem_destroy(&chopstick[i]);

    return 0;
}

void* philosopher(void* who) {
    int no = (int)*((int*)who);  // 获取哲学家的编号
    int i = 0;

    for (i = 0; i < 10; i++) {
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                           \n", 'A' + no); fflush(stdout);
        randomDelay();

        // 哲学家尝试拿左边的筷子
        sem_wait(&chopstick[no]);
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);
        randomDelay();

        // 哲学家尝试拿右边的筷子
        sem_wait(&chopstick[(no + 1) % CHAIRNUM]);
        gotoxy(1, no * 4 + 2);
        printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);

        // 关键区域：哲学家进餐
        randomDelay();
        timeUsed[no]++;
        timeUsed[(no + 1) % CHAIRNUM]++;
        gotoxy(no * 10 + 1, CHAIRNUM * 4 + 1);
        printf("CH[%d]=%d ", no, timeUsed[no]);
        gotoxy(((no + 1) % CHAIRNUM) * 10 + 1, CHAIRNUM * 4 + 1);
        printf("CH[%d]=%d ", (no + 1) % CHAIRNUM, timeUsed[(no + 1) % CHAIRNUM]);
        gotoxy(no * 10 + 1, CHAIRNUM * 4 + 2);
        printf("  [%c]=%d ", 'A' + no, i);

        // 清理屏幕
        gotoxy(1, no * 4 + 1);
        printf("                                                                 \n");
        gotoxy(1, no * 4 + 2);
        printf("                                                                 \n");

        randomDelay();

        // 哲学家放下左边的筷子
        sem_post(&chopstick[no]);
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c drops a chopstick on the left side...        \n", 'A' + no); fflush(stdout);

        // 哲学家放下右边的筷子
        sem_post(&chopstick[(no + 1) % CHAIRNUM]);
        gotoxy(1, no * 4 + 2);
        printf("Mr. %c drops a chopstick on the right side...       \n", 'A' + no); fflush(stdout);

        randomDelay();
        gotoxy(1, no * 4 + 2);
        printf("                                                                 \n");
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is chewing food...                             \n", 'A' + no); fflush(stdout);
        randomDelay();
    }
    gotoxy(1, no * 4 + 1);
    printf("Mr. %c is full...                                         \n", 'A' + no); fflush(stdout);
    pthread_exit(0);
}

void randomDelay(void) {
    // This function provides a delay which slows the process down so we can see what happens
    srand(time(NULL));
    int stime = ((rand() % 2000) + 100) * 1000;
    usleep(stime);
}
