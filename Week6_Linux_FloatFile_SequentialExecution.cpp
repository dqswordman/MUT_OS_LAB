#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

float sharedVar1 = 0.0, sharedVar2 = 0.0;

pthread_mutex_t lock;
pthread_cond_t cond;

int running = 1;

void *threadFunctionA(void *arg);
void *threadFunctionB(void *arg);
void *threadFunctionC(void *arg);

int main() {
    pthread_t tidA, tidB, tidC;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&tidA, NULL, threadFunctionA, NULL);
    pthread_create(&tidB, NULL, threadFunctionB, NULL);
    pthread_create(&tidC, NULL, threadFunctionC, NULL);

    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
    pthread_join(tidC, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}

void *threadFunctionA(void *arg) {
    while (running) {
        pthread_mutex_lock(&lock);


        printf("请输入两个浮点值：\n");
        scanf("%f %f", &sharedVar1, &sharedVar2);
        if (sharedVar1 > 100.0) running = 0;

        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);

        usleep(100000);
    }
    return NULL;
}

void *threadFunctionB(void *arg) {
    while (running) {
        usleep(1000000);
        /*pthread_mutex_lock(&lock);
        while (!flagB) {
            pthread_cond_wait(&cond, &lock);
        }*/

        if (running) printf("差值：%f\n", sharedVar2 - sharedVar1);


        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);

        usleep(100000);
    }
    return NULL;
}

void *threadFunctionC(void *arg) {
    while (running) {
        usleep(1000000);
        /*pthread_mutex_lock(&lock);
        while (!flagC) {
            pthread_cond_wait(&cond, &lock);
        }*/

        if (running) printf("当前值：Var1 = %f, Var2 = %f\n", sharedVar1, sharedVar2);


        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);

        usleep(100000);
    }
    return NULL;
}
