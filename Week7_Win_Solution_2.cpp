/*
奇数编号哲学家：先拿左边的筷子，再拿右边的筷子。
偶数编号哲学家：先拿右边的筷子，再拿左边的筷子。
不对称的筷子拿取顺序：
偶数编号的哲学家（如哲学家B、D）：先拿右边的筷子，再拿左边的筷子。
奇数编号的哲学家（如哲学家A、C、E）：先拿左边的筷子，再拿右边的筷子。
目的：通过这种方式打破对称性，减少所有哲学家都同时等待对方的资源的情况，降低死锁的可能性。
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

        if (no % 2 == 0) {  // 偶数编号的哲学家，先拿右边再拿左边
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
            gotoxy(1, no * 4 + 1);
            printf("Mr. %c is taking a chopstick on the right side...    \n", 'A' + no); fflush(stdout);

            randomDelay();  // 引入延时，模拟占有并等待情况

            sem_wait(chopstick[no]);  // 拿左边的筷子
            gotoxy(1, no * 4 + 2);
            printf("Mr. %c is taking a chopstick on the left side...\n", 'A' + no); fflush(stdout);
        }
        else {  // 奇数编号的哲学家，先拿左边再拿右边
            sem_wait(chopstick[no]);  // 拿左边的筷子
            gotoxy(1, no * 4 + 1);
            printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);

            randomDelay();  // 引入延时，模拟占有并等待情况

            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // 拿右边的筷子
            gotoxy(1, no * 4 + 2);
            printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);
        }

        // Critical Section
        gotoxy(1, no * 4 + 3);
        printf("Mr. %c is eating...                               \n", 'A' + no); fflush(stdout);
        randomDelay();  // 吃饭时间
        timeUsed[no]++;

        // 释放筷子
        sem_signal(chopstick[no]);
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating...                          \n", 'A' + no); fflush(stdout);
        randomDelay();  // 哲学家思考的时间
    }
    return 0;
}
