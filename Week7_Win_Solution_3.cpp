/*
方案三：强制每个哲学家同时拿起两只筷子
在这个方案中，我们要确保每个哲学家只能在同时拿到两只筷子时才能进入“吃饭”状态。这意味着哲学家必须同时获取两只筷子的资源，否则就不能进入“进餐”状态。

实现方案：
使用单个信号量 控制哲学家同时拿起两只筷子。这相当于为每个哲学家设计一种“原子操作”机制，确保他们只能在获取到两只筷子时才执行接下来的进餐操作。
打破对称性，避免哲学家卡住在等待单个筷子的状态。

同时拿起两只筷子：

在代码中，我们保证哲学家必须同时拿到两只筷子（左边和右边）才能开始吃饭。
偶数编号哲学家（如B、D）先拿右边筷子，再拿左边筷子。
奇数编号哲学家（如A、C、E）先拿左边筷子，再拿右边筷子。
原子操作：

我们通过确保哲学家只能在同时获取两只筷子时才进入“吃饭”状态，从而防止哲学家卡在等待单个筷子的状态下，这样避免了死锁。
*/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "C:\Users\Du\Downloads\lab\lab\conio.h"  // 根据实际路径调整

#define CHAIRNUM 5

HANDLE chopstick[CHAIRNUM];  // 筷子的信号量
int timeUsed[CHAIRNUM];

void randomDelay(void);
void shortDelay(void);
DWORD WINAPI philosopher(LPVOID who);

int main() {
    HANDLE th[CHAIRNUM];      // 哲学家线程句柄
    DWORD tid[CHAIRNUM];      // 线程ID
    int param[CHAIRNUM];      // 参数数组
    int i;

    clrscr();  // 清屏
    srand((unsigned)time(NULL));  // 初始化随机数种子

    // 初始化信号量
    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(NULL, 1, 1, NULL);  // 初始信号量值为1
        param[i] = i;
        timeUsed[i] = 0;
    }

    // 创建哲学家线程
    for (i = 0; i < CHAIRNUM; i++) {
        th[i] = CreateThread(NULL, 0, philosopher, &param[i], 0, &tid[i]);
        Sleep(100);  // 避免同时启动所有线程，等待100毫秒
    }

    // 等待所有线程完成
    for (i = 0; i < CHAIRNUM; i++) {
        WaitForSingleObject(th[i], INFINITE);
        CloseHandle(th[i]);
    }

    // 销毁信号量
    for (i = 0; i < CHAIRNUM; i++) {
        CloseHandle(chopstick[i]);
    }

    return 0;
}

// 哲学家线程函数
DWORD WINAPI philosopher(LPVOID who) {
    int no = *((int*)who);
    int i = 0;

    for (i = 0; i < 10; i++) {
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...\n", 'A' + no);
        fflush(stdout);
        randomDelay();  // 模拟思考时间

        // 尝试拿起第一根筷子
        if (WaitForSingleObject(chopstick[no], INFINITE) == WAIT_OBJECT_0) {
            // 尝试拿起第二根筷子
            if (WaitForSingleObject(chopstick[(no + 1) % CHAIRNUM], INFINITE) == WAIT_OBJECT_0) {
                // 成功拿起两根筷子，开始吃饭
                gotoxy(1, no * 4 + 1);
                printf("Mr. %c has taken both chopsticks and is eating...\n", 'A' + no);
                fflush(stdout);
                randomDelay();  // 模拟吃饭时间

                // 吃完后放下筷子
                ReleaseSemaphore(chopstick[no], 1, NULL);
                ReleaseSemaphore(chopstick[(no + 1) % CHAIRNUM], 1, NULL);

                gotoxy(1, no * 4 + 1);
                printf("Mr. %c drops both chopsticks...\n", 'A' + no);
                fflush(stdout);
            }
            else {
                // 如果无法拿起第二根筷子，放下第一根筷子并短暂延迟
                ReleaseSemaphore(chopstick[no], 1, NULL);
                shortDelay();  // 短暂延迟后再试
            }
        }

        randomDelay();  // 模拟吃完后继续思考
    }

    gotoxy(1, no * 4 + 1);
    printf("Mr. %c is full...\n", 'A' + no);
    fflush(stdout);

    return 0;
}

// 随机延迟函数，模拟思考或吃饭时间
void randomDelay(void) {
    int stime = (rand() % 500) + 10;  // 随机延迟10-510毫秒
    Sleep(stime);
}

// 短暂延迟函数，避免频繁争抢
void shortDelay(void) {
    Sleep(10);  // 短暂延迟10毫秒
}
