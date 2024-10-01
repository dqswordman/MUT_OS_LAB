#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "C:\Users\Administrator\Desktop\lab\lab\winconio.h"

#define CHAIRNUM 5  // ����5������
#define PHILO_NUM 4 // ֻ����4����ѧ�Ҳ������

HANDLE chopstick[CHAIRNUM];  // ���ӵ��ź���
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

    // ��ʼ��5�����ӵ��ź���
    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(NULL, 1, 1, NULL);
        if (chopstick[i] == NULL) {
            printf("Failed to create semaphore for chopstick[%d]\n", i);
            return 1;  // ������������
        }
        timeUsed[i] = 0;
    }

    // ����4����ѧ���߳�
    for (i = 0; i < PHILO_NUM; i++) {
        param[i] = i;  // ��ѧ�ҵı��
        th[i] = CreateThread(
            NULL,                // Default security attributes
            0,                   // Default stack size
            philosopher,         // Thread function
            (void*)&param[i],    // Thread function parameter
            0,                   // Default creation flag
            &tid[i]);            // Thread ID returned.
        if (th[i] == NULL) {
            printf("Failed to create thread for philosopher %d\n", i);
            return 1;  // ������������
        }
    }

    // �ȴ�������ѧ���߳̽���
    for (i = 0; i < PHILO_NUM; i++) {
        if (th[i] != NULL) {
            WaitForSingleObject(th[i], INFINITE);
        }
    }

    // �ر��ź���
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

    no = (int)*((int*)who);  // ��ȡ��ѧ�ҵı��

    for (i = 0; i < 10; i++) {  // ��ѧ�ҳ�10��
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...                           \n", 'A' + no); fflush(stdout);
        randomDelay();

        // ��ѧ�ҳ���������ߵĿ���
        sem_wait(chopstick[no]);
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);

        randomDelay();  // ģ���ӳ�

        // ��ѧ�ҳ��������ұߵĿ���
        sem_wait(chopstick[(no + 1) % CHAIRNUM]);
        gotoxy(1, no * 4 + 2);
        printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);

        // ��ѧ�ҽ���
        gotoxy(1, no * 4 + 3);
        printf("Mr. %c is eating...                               \n", 'A' + no); fflush(stdout);
        randomDelay();  // ����ʱ��
        timeUsed[no]++;

        // ��ѧ�ҷ��¿���
        sem_signal(chopstick[no]);  // ������ߵĿ���
        sem_signal(chopstick[(no + 1) % CHAIRNUM]);  // �����ұߵĿ���

        gotoxy(1, no * 4 + 1);
        printf("Mr. %c finished eating...                          \n", 'A' + no); fflush(stdout);
        randomDelay();  // ˼����ʱ��
    }
    return 0;
}
