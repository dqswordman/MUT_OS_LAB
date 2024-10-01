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
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "C:\Users\Administrator\Desktop\lab\lab\winconio.h"

#define CHAIRNUM 5

HANDLE chopstick[CHAIRNUM];
int timeUsed[CHAIRNUM];

void randomDelay(void);
DWORD WINAPI philosopher(LPVOID who);
DWORD sem_wait(HANDLE sem);
DWORD sem_signal(HANDLE sem);

int main(void) {
    int i;
    DWORD tid[CHAIRNUM];                // Thread ID
    HANDLE th[CHAIRNUM];                // Thread Handle
    int param[CHAIRNUM];

    clrscr();

    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(
            NULL,   // default security attributes
            1,      // initial count
            1,      // maximum count
            NULL);  // unnamed semaphore
        param[i] = i;
        timeUsed[i] = 0;
    }

    // Create n threads
    for (i = 0; i < CHAIRNUM; i++)
        th[i] = CreateThread(
            NULL,                // Default security attributes
            0,                   // Default stack size
            philosopher,         // Thread function
            (void*)&param[i],   // Thread function parameter
            0,                   // Default creation flag
            &tid[i]);            // Thread ID returned.

    // Wait until all threads finish
    for (i = 0; i < CHAIRNUM; i++)
        if (th[i] != NULL)
            WaitForSingleObject(th[i], INFINITE);

    for (i = 0; i < CHAIRNUM; i++)
        CloseHandle(chopstick[i]);

    return 0;
}

void randomDelay(void) {
    int stime = ((rand() % 2000) + 100);
    Sleep(stime);
}

DWORD sem_wait(HANDLE sem) {
    return WaitForSingleObject(sem, INFINITE);
}

DWORD sem_signal(HANDLE sem) {
    return ReleaseSemaphore(sem, 1, NULL);
}

DWORD WINAPI philosopher(LPVOID who) {
    int no, i = 0;

    no = (int)*((int*)who);

    for (i = 0; i < 10; i++) {
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                           \n", 'A' + no); fflush(stdout);
        randomDelay();

        // 同时获取两只筷子，确保原子操作
        if (no % 2 == 0) {  // 偶数编号哲学家
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
            sem_wait(chopstick[no]);  // 拿左边的筷子
        }
        else {  // 奇数编号哲学家
            sem_wait(chopstick[no]);  // 拿左边的筷子
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
        }

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c has taken both chopsticks and is eating...    \n", 'A' + no); fflush(stdout);

        // Critical Section
        randomDelay();  // 吃饭时间
        timeUsed[no]++;

        // 释放两只筷子
        sem_signal(chopstick[no]);
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating and is thinking...            \n", 'A' + no); fflush(stdout);
        randomDelay();  // 哲学家思考时间
    }
    return 0;
}
