#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TERMINATE_VALUE -1  // 定义终止值

// 共享数据结构
struct shared_data {
    int val1;
    int val2;
};

// 共享数据实例
struct shared_data shm;

// 标志变量，用于控制线程运行状态
int terminate = 0;

// 线程A函数：获取用户输入
void *threadFunctionA(void *arg) {
    while (1) {
        printf("Enter two integers (first < 0 to quit): ");
        if (scanf("%d %d", &shm.val1, &shm.val2) != 2) {
            printf("Invalid input. Please enter two integer numbers.\n");
            while (getchar() != '\n'); // 清空输入缓冲区
            continue;
        }
        if (shm.val1 < 0) {
            terminate = 1; // 设置退出标志
            break;
        }
    }
    pthread_exit(0);
}

// 线程B函数：计算和
void *threadFunctionB(void *arg) {
    while (!terminate) {
        sleep(2); // 等待2秒
        if (shm.val1 < 0) break; // 检查终止条件
        printf("B: Sum is %d\n", shm.val1 + shm.val2);
    }
    pthread_exit(0);
}

// 线程C函数：计算积
void *threadFunctionC(void *arg) {
    while (!terminate) {
        sleep(3); // 等待3秒
        if (shm.val1 < 0) break; // 检查终止条件
        printf("C: Product is %d\n", shm.val1 * shm.val2);
    }
    pthread_exit(0);
}

int main() {
    pthread_t tidA, tidB, tidC;

    shm.val1 = 0;
    shm.val2 = 0;

    // 创建线程A、B和C
    pthread_create(&tidA, NULL, threadFunctionA, NULL);
    pthread_create(&tidB, NULL, threadFunctionB, NULL);
    pthread_create(&tidC, NULL, threadFunctionC, NULL);

    // 等待线程A、B和C结束
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
    pthread_join(tidC, NULL);

    return 0;
}
