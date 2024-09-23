#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 控制标志变量，确保按顺序执行
int flagA = 1;  // 初始为线程A可以执行
int flagB = 0;  // 线程B等待
int flagC = 0;  // 线程C等待

pthread_mutex_t lock;  // 定义一个互斥锁
pthread_cond_t cond;   // 定义条件变量，用来协调线程

int running = 1;  // 控制线程运行的标志

// 线程函数声明
void *threadFunctionA(void *arg);
void *threadFunctionB(void *arg);
void *threadFunctionC(void *arg);

int main() {
    pthread_t tidA, tidB, tidC;

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    // 创建线程A、B、C
    pthread_create(&tidA, NULL, threadFunctionA, NULL);
    pthread_create(&tidB, NULL, threadFunctionB, NULL);
    pthread_create(&tidC, NULL, threadFunctionC, NULL);

    // 等待线程A、B、C结束
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
    pthread_join(tidC, NULL);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}

// 线程A：负责用户输入
void *threadFunctionA(void *arg) {
    int userInput = 0;
    while (running) {
        pthread_mutex_lock(&lock);  // 锁住互斥锁
        while (!flagA) {  // 等待线程A的执行时机
            pthread_cond_wait(&cond, &lock);
        }
        // 执行线程A的操作
        printf("线程A正在运行：请输入一个整数（输入负数停止）：\n");
        scanf("%d", &userInput);
        if (userInput < 0) {
            running = 0;  // 终止程序
        }
        // 线程A执行完毕，轮到线程B
        flagA = 0;
        flagB = 1;
        pthread_cond_broadcast(&cond);  // 通知其他线程
        pthread_mutex_unlock(&lock);    // 解锁互斥锁

        usleep(100000);  // 避免过度占用CPU
    }
    return NULL;
}

// 线程B：负责计算
void *threadFunctionB(void *arg) {
    while (running) {
        pthread_mutex_lock(&lock);  // 锁住互斥锁
        while (!flagB) {  // 等待线程B的执行时机
            pthread_cond_wait(&cond, &lock);
        }
        if (running) {
            printf("线程B正在运行...\n");
        }
        // 线程B执行完毕，轮到线程C
        flagB = 0;
        flagC = 1;
        pthread_cond_broadcast(&cond);  // 通知其他线程
        pthread_mutex_unlock(&lock);    // 解锁互斥锁

        usleep(100000);  // 避免过度占用CPU
    }
    return NULL;
}

// 线程C：负责其他操作
void *threadFunctionC(void *arg) {
    while (running) {
        pthread_mutex_lock(&lock);  // 锁住互斥锁
        while (!flagC) {  // 等待线程C的执行时机
            pthread_cond_wait(&cond, &lock);
        }
        if (running) {
            printf("线程C正在运行...\n");
        }
        // 线程C执行完毕，轮到线程A
        flagC = 0;
        flagA = 1;
        pthread_cond_broadcast(&cond);  // 通知其他线程
        pthread_mutex_unlock(&lock);    // 解锁互斥锁

        usleep(100000);  // 避免过度占用CPU
    }
    return NULL;
}

