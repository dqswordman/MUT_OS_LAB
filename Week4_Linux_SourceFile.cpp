#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 声明三个独立的线程函数
void *threadFunctionA(void *arg);
void *threadFunctionB(void *arg);
void *threadFunctionC(void *arg);

int main(void) {
    pthread_t tid1, tid2, tid3;  // 线程ID

    // 创建3个线程
    pthread_create(&tid1, NULL, threadFunctionA, NULL);
    pthread_create(&tid2, NULL, threadFunctionB, NULL);
    pthread_create(&tid3, NULL, threadFunctionC, NULL);

    // 等待所有线程完成
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    return 0;
}

void *threadFunctionA(void *arg) {
    printf("Running from thread A\n");
    sleep(1);  // 模拟线程运行延迟
    pthread_exit(0);
}

void *threadFunctionB(void *arg) {
    printf("Running from thread B\n");
    sleep(1);  // 模拟线程运行延迟
    pthread_exit(0);
}

void *threadFunctionC(void *arg) {
    printf("Running from thread C\n");
    sleep(1);  // 模拟线程运行延迟
    pthread_exit(0);
}

