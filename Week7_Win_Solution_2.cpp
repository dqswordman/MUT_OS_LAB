/*
���������ѧ�ң�������ߵĿ��ӣ������ұߵĿ��ӡ�
ż�������ѧ�ң������ұߵĿ��ӣ�������ߵĿ��ӡ�
���ԳƵĿ�����ȡ˳��
ż����ŵ���ѧ�ң�����ѧ��B��D���������ұߵĿ��ӣ�������ߵĿ��ӡ�
������ŵ���ѧ�ң�����ѧ��A��C��E����������ߵĿ��ӣ������ұߵĿ��ӡ�
Ŀ�ģ�ͨ�����ַ�ʽ���ƶԳ��ԣ�����������ѧ�Ҷ�ͬʱ�ȴ��Է�����Դ����������������Ŀ����ԡ�
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

        if (no % 2 == 0) {  // ż����ŵ���ѧ�ң������ұ��������
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // ���ұߵĿ���
            gotoxy(1, no * 4 + 1);
            printf("Mr. %c is taking a chopstick on the right side...    \n", 'A' + no); fflush(stdout);

            randomDelay();  // ������ʱ��ģ��ռ�в��ȴ����

            sem_wait(chopstick[no]);  // ����ߵĿ���
            gotoxy(1, no * 4 + 2);
            printf("Mr. %c is taking a chopstick on the left side...\n", 'A' + no); fflush(stdout);
        }
        else {  // ������ŵ���ѧ�ң�������������ұ�
            sem_wait(chopstick[no]);  // ����ߵĿ���
            gotoxy(1, no * 4 + 1);
            printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);

            randomDelay();  // ������ʱ��ģ��ռ�в��ȴ����

            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // ���ұߵĿ���
            gotoxy(1, no * 4 + 2);
            printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);
        }

        // Critical Section
        gotoxy(1, no * 4 + 3);
        printf("Mr. %c is eating...                               \n", 'A' + no); fflush(stdout);
        randomDelay();  // �Է�ʱ��
        timeUsed[no]++;

        // �ͷſ���
        sem_signal(chopstick[no]);
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating...                          \n", 'A' + no); fflush(stdout);
        randomDelay();  // ��ѧ��˼����ʱ��
    }
    return 0;
}
