#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "C:\Users\Administrator\Desktop\lab\lab\winconio.h"

#define CHAIRNUM 5

HANDLE chopstick[CHAIRNUM];
int timeUsed[CHAIRNUM];

// 新增信号量，限制最多4个哲学家同时就餐
HANDLE maxSeats;

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
    maxSeats = CreateSemaphore(
        NULL,   // default security attributes
        4,      // initial count (4 philosophers can attempt to eat)
        4,      // maximum count
        NULL);  // unnamed semaphore

    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(NULL, 1, 1, NULL);
        if (chopstick[i] == NULL) {
            printf("Failed to create semaphore for chopstick[%d]\n", i);
            return 1;  // 或其他错误处理
        }
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

    // 关闭信号量
    CloseHandle(maxSeats);
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

        // 获取信号量，确保最多4个哲学家同时拿筷子
        sem_wait(maxSeats);

        sem_wait(chopstick[no]);  // 拿左边的筷子
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);

        randomDelay();  // 引入延时，模拟占有并等待情况

        sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
        gotoxy(1, no * 4 + 2);
        printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);

        // Critical Section
        gotoxy(1, no * 4 + 3);
        printf("Mr. %c is eating...                               \n", 'A' + no); fflush(stdout);
        randomDelay();  // 吃饭时间
        timeUsed[no]++;

        // 释放筷子
        sem_signal(chopstick[no]);
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);

        // 释放座位信号量
        sem_signal(maxSeats);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating...                          \n", 'A' + no); fflush(stdout);
        randomDelay();  // 哲学家思考的时间
    }
    return 0;
}
