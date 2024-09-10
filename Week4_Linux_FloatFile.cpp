#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 共享变量
float sharedVar1 = 0.0;
float sharedVar2 = 0.0;
int running = 1;  // 控制线程运行的标志

// 线程函数声明
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

// 线程A：等待用户输入两个浮点值
void *threadFunctionA(void *arg) {
    while (1) {
        printf("请输入两个浮点值：\n");
        scanf("%f %f", &sharedVar1, &sharedVar2);
        if (sharedVar1 > 100.0) {
            running = 0;  // 设置终止标志
            break;
        }
    }
    pthread_exit(0);
}

// 线程B：计算并显示两个共享变量的差
void *threadFunctionB(void *arg) {
    while (running) {
        printf("差值：%f\n", sharedVar2 - sharedVar1);
        sleep(2);  // 等待2秒
        if (sharedVar1 > 100.0) {
            break;
        }
    }
    pthread_exit(0);
}

// 线程C：显示两个共享变量的值
void *threadFunctionC(void *arg) {
    while (running) {
        printf("当前值：Var1 = %f, Var2 = %f\n", sharedVar1, sharedVar2);
        sleep(3);  // 等待3秒
        if (sharedVar1 > 100.0) {
            break;
        }
    }
    pthread_exit(0);
}
