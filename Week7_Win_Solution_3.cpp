/*
��������ǿ��ÿ����ѧ��ͬʱ������ֻ����
����������У�����Ҫȷ��ÿ����ѧ��ֻ����ͬʱ�õ���ֻ����ʱ���ܽ��롰�Է���״̬������ζ����ѧ�ұ���ͬʱ��ȡ��ֻ���ӵ���Դ������Ͳ��ܽ��롰���͡�״̬��

ʵ�ַ�����
ʹ�õ����ź��� ������ѧ��ͬʱ������ֻ���ӡ����൱��Ϊÿ����ѧ�����һ�֡�ԭ�Ӳ��������ƣ�ȷ������ֻ���ڻ�ȡ����ֻ����ʱ��ִ�н������Ľ��Ͳ�����
���ƶԳ��ԣ�������ѧ�ҿ�ס�ڵȴ��������ӵ�״̬��

ͬʱ������ֻ���ӣ�

�ڴ����У����Ǳ�֤��ѧ�ұ���ͬʱ�õ���ֻ���ӣ���ߺ��ұߣ����ܿ�ʼ�Է���
ż�������ѧ�ң���B��D�������ұ߿��ӣ�������߿��ӡ�
���������ѧ�ң���A��C��E��������߿��ӣ������ұ߿��ӡ�
ԭ�Ӳ�����

����ͨ��ȷ����ѧ��ֻ����ͬʱ��ȡ��ֻ����ʱ�Ž��롰�Է���״̬���Ӷ���ֹ��ѧ�ҿ��ڵȴ��������ӵ�״̬�£�����������������
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

        // ͬʱ��ȡ��ֻ���ӣ�ȷ��ԭ�Ӳ���
        if (no % 2 == 0) {  // ż�������ѧ��
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // ���ұߵĿ���
            sem_wait(chopstick[no]);  // ����ߵĿ���
        }
        else {  // ���������ѧ��
            sem_wait(chopstick[no]);  // ����ߵĿ���
            sem_wait(chopstick[(no + 1) % CHAIRNUM]);  // ���ұߵĿ���
        }

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c has taken both chopsticks and is eating...    \n", 'A' + no); fflush(stdout);

        // Critical Section
        randomDelay();  // �Է�ʱ��
        timeUsed[no]++;

        // �ͷ���ֻ����
        sem_signal(chopstick[no]);
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating and is thinking...            \n", 'A' + no); fflush(stdout);
        randomDelay();  // ��ѧ��˼��ʱ��
    }
    return 0;
}
