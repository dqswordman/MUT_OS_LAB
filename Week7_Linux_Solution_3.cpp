#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // for usleep()
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "/home/miic1225/Downloads/lab/conio.h"  // 根据实际路径调整

#define CHAIRNUM 5

sem_t chopstick[CHAIRNUM];  // 筷子的信号量
int timeUsed[CHAIRNUM];

void randomDelay(void);
void shortDelay(void);
void *philosopher(void *who);

int main() {
    pthread_t tid[CHAIRNUM];
    int param[CHAIRNUM];
    int i;

    clrscr();  // 清屏
    srand((unsigned)time(NULL));  // 初始化随机数种子

    // 初始化信号量
    for (i = 0; i < CHAIRNUM; i++) {
        sem_init(&chopstick[i], 0, 1);  // 初始化信号量，值为1
        param[i] = i;
        timeUsed[i] = 0;
    }

    // 创建哲学家线程
    for (i = 0; i < CHAIRNUM; i++) {
        pthread_create(&tid[i], NULL, philosopher, &param[i]);
        usleep(100 * 1000);  // 避免线程同时启动，等待100毫秒
    }

    // 等待所有线程完成
    for (i = 0; i < CHAIRNUM; i++) {
        pthread_join(tid[i], NULL);
    }

    // 销毁信号量
    for (i = 0; i < CHAIRNUM; i++) {
        sem_destroy(&chopstick[i]);
    }

    return 0;
}

// 哲学家线程函数
void *philosopher(void *who) {
    int no = *((int *)who);
    int i = 0;

    for (i = 0; i < 10; i++) {
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                                    \n", 'A' + no);
        fflush(stdout);
        randomDelay();  // 模拟思考时间

        // 尝试拿起第一根筷子
        if (sem_wait(&chopstick[no]) == 0) {
            // 尝试拿起第二根筷子
            if (sem_wait(&chopstick[(no + 1) % CHAIRNUM]) == 0) {
                // 成功拿起两根筷子，开始吃饭
                gotoxy(1, no * 4 + 1);
                printf("Mr. %c has taken both chopsticks and is eating...        \n", 'A' + no);
                fflush(stdout);
                randomDelay();  // 模拟吃饭时间

                // 吃完后放下筷子
                sem_post(&chopstick[no]);
                sem_post(&chopstick[(no + 1) % CHAIRNUM]);

                gotoxy(1, no * 4 + 1);
                printf("Mr. %c drops both chopsticks...                          \n", 'A' + no);
                fflush(stdout);
            } else {
                // 如果无法拿起第二根筷子，放下第一根筷子并短暂延迟
                sem_post(&chopstick[no]);
                shortDelay();  // 短暂延迟后再试
            }
        }

        randomDelay();  // 模拟吃完后继续思考
    }

    gotoxy(1, no * 4 + 1);
    printf("Mr. %c is full...                                         \n", 'A' + no);
    fflush(stdout);

    pthread_exit(0);
}

// 随机延迟函数，模拟思考或吃饭时间
void randomDelay(void) {
    int stime = (rand() % 500) + 10;  // 随机延迟10-510毫秒
    usleep(stime * 1000);  // usleep 使用微秒
}

// 短暂延迟函数，避免频繁争抢
void shortDelay(void) {
    usleep(10 * 1000);  // 短暂延迟10毫秒
}
