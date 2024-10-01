#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "C:\Users\Administrator\Desktop\lab\lab\winconio.h"

#define CHAIRNUM 5  // 保留5把椅子
#define PHILO_NUM 4 // 只允许4个哲学家参与进餐

HANDLE chopstick[CHAIRNUM];  // 筷子的信号量
int timeUsed[CHAIRNUM];

void randomDelay(void);
DWORD WINAPI philosopher(LPVOID who);
DWORD sem_wait(HANDLE sem);
DWORD sem_signal(HANDLE sem);

int main(void) {
    int i;
    DWORD tid[PHILO_NUM];                // Thread ID
    HANDLE th[PHILO_NUM];                // Thread Handle
    int param[PHILO_NUM];

    clrscr();

    // 初始化5根筷子的信号量
    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(NULL, 1, 1, NULL);
        if (chopstick[i] == NULL) {
            printf("Failed to create semaphore for chopstick[%d]\n", i);
            return 1;  // 或其他错误处理
        }
        timeUsed[i] = 0;
    }

    // 创建4个哲学家线程
    for (i = 0; i < PHILO_NUM; i++) {
        param[i] = i;  // 哲学家的编号
        th[i] = CreateThread(
            NULL,                // Default security attributes
            0,                   // Default stack size
            philosopher,         // Thread function
            (void*)&param[i],    // Thread function parameter
            0,                   // Default creation flag
            &tid[i]);            // Thread ID returned.
        if (th[i] == NULL) {
            printf("Failed to create thread for philosopher %d\n", i);
            return 1;  // 或其他错误处理
        }
    }

    // 等待所有哲学家线程结束
    for (i = 0; i < PHILO_NUM; i++) {
        if (th[i] != NULL) {
            WaitForSingleObject(th[i], INFINITE);
        }
    }

    // 关闭信号量
    for (i = 0; i < CHAIRNUM; i++) {
        CloseHandle(chopstick[i]);
    }

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

    no = (int)*((int*)who);  // 获取哲学家的编号

    for (i = 0; i < 10; i++) {  // 哲学家吃10次
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                           \n", 'A' + no); fflush(stdout);
        randomDelay();

        // 哲学家尝试拿起左边的筷子
        sem_wait(chopstick[no]);
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);

        randomDelay();  // 模拟延迟

        // 哲学家尝试拿起右边的筷子
        sem_wait(chopstick[(no + 1) % CHAIRNUM]);
        gotoxy(1, no * 4 + 2);
        printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);

        // 哲学家进餐
        gotoxy(1, no * 4 + 3);
        printf("Mr. %c is eating...                               \n", 'A' + no); fflush(stdout);
        randomDelay();  // 进餐时间
        timeUsed[no]++;

        // 哲学家放下筷子
        sem_signal(chopstick[no]);  // 放下左边的筷子
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);  // 放下右边的筷子

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating...                          \n", 'A' + no); fflush(stdout);
        randomDelay();  // 思考的时间
    }
    return 0;
}
