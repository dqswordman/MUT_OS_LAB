#include <iostream>
#include <pthread.h>
#include <unistd.h>

float sharedVar1 = 0.0;
float sharedVar2 = 0.0;
bool running = true;  // 控制线程运行的标志
pthread_mutex_t lock;

// 线程函数声明
void* threadFunctionA(void* arg);
void* threadFunctionB(void* arg);
void* threadFunctionC(void* arg);

int main() {
    pthread_t tid1, tid2, tid3;  // 线程ID

    // 初始化互斥锁
    if (pthread_mutex_init(&lock, NULL) != 0) {
        std::cerr << "初始化互斥锁失败\n";
        return 1;
    }

    // 创建3个线程
    if (pthread_create(&tid1, NULL, threadFunctionA, NULL) != 0 ||
        pthread_create(&tid2, NULL, threadFunctionB, NULL) != 0 ||
        pthread_create(&tid3, NULL, threadFunctionC, NULL) != 0) {
        std::cerr << "线程创建失败\n";
        return 1;
    }

    // 等待所有线程完成
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    // 销毁互斥锁
    pthread_mutex_destroy(&lock);

    return 0;
}

// 线程A：等待用户输入两个浮点值
void* threadFunctionA(void* arg) {
    while (true) {
        float localVar1, localVar2;
        std::cout << "请输入两个浮点值：\n";
        std::cin >> localVar1 >> localVar2;

        pthread_mutex_lock(&lock);
        sharedVar1 = localVar1;
        sharedVar2 = localVar2;
        pthread_mutex_unlock(&lock);

        if (localVar1 > 100.0) {
            running = false;  // 设置终止标志
            break;
        }
    }
    pthread_exit(0);
}

// 线程B：计算并显示两个共享变量的差
void* threadFunctionB(void* arg) {
    while (running) {
        float localVar1, localVar2, difference;

        pthread_mutex_lock(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        pthread_mutex_unlock(&lock);

        difference = localVar2 - localVar1;
        std::cout << "差值：" << difference << std::endl;

        sleep(2);  // 等待2秒

        if (!running) {
            break;
        }
    }
    pthread_exit(0);
}

// 线程C：显示两个共享变量的值
void* threadFunctionC(void* arg) {
    while (running) {
        float localVar1, localVar2;

        pthread_mutex_lock(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        pthread_mutex_unlock(&lock);

        std::cout << "当前值：Var1 = " << localVar1 << ", Var2 = " << localVar2 << std::endl;

        sleep(3);  // 等待3秒

        if (!running) {
            break;
        }
    }
    pthread_exit(0);
}
