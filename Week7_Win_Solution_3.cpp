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
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "C:\Users\Du\Downloads\lab\lab\conio.h"  // ����ʵ��·������

#define CHAIRNUM 5

HANDLE chopstick[CHAIRNUM];  // ���ӵ��ź���
int timeUsed[CHAIRNUM];

void randomDelay(void);
void shortDelay(void);
DWORD WINAPI philosopher(LPVOID who);

int main() {
    HANDLE th[CHAIRNUM];      // ��ѧ���߳̾��
    DWORD tid[CHAIRNUM];      // �߳�ID
    int param[CHAIRNUM];      // ��������
    int i;

    clrscr();  // ����
    srand((unsigned)time(NULL));  // ��ʼ�����������

    // ��ʼ���ź���
    for (i = 0; i < CHAIRNUM; i++) {
        chopstick[i] = CreateSemaphore(NULL, 1, 1, NULL);  // ��ʼ�ź���ֵΪ1
        param[i] = i;
        timeUsed[i] = 0;
    }

    // ������ѧ���߳�
    for (i = 0; i < CHAIRNUM; i++) {
        th[i] = CreateThread(NULL, 0, philosopher, &param[i], 0, &tid[i]);
        Sleep(100);  // ����ͬʱ���������̣߳��ȴ�100����
    }

    // �ȴ������߳����
    for (i = 0; i < CHAIRNUM; i++) {
        WaitForSingleObject(th[i], INFINITE);
        CloseHandle(th[i]);
    }

    // �����ź���
    for (i = 0; i < CHAIRNUM; i++) {
        CloseHandle(chopstick[i]);
    }

    return 0;
}

// ��ѧ���̺߳���
DWORD WINAPI philosopher(LPVOID who) {
    int no = *((int*)who);
    int i = 0;

    for (i = 0; i < 10; i++) {
        gotoxy(1, no * 4 + 1);
        printf("Mr. %c is thinking...\n", 'A' + no);
        fflush(stdout);
        randomDelay();  // ģ��˼��ʱ��

        // ���������һ������
        if (WaitForSingleObject(chopstick[no], INFINITE) == WAIT_OBJECT_0) {
            // ��������ڶ�������
            if (WaitForSingleObject(chopstick[(no + 1) % CHAIRNUM], INFINITE) == WAIT_OBJECT_0) {
                // �ɹ������������ӣ���ʼ�Է�
                gotoxy(1, no * 4 + 1);
                printf("Mr. %c has taken both chopsticks and is eating...\n", 'A' + no);
                fflush(stdout);
                randomDelay();  // ģ��Է�ʱ��

                // �������¿���
                ReleaseSemaphore(chopstick[no], 1, NULL);
                ReleaseSemaphore(chopstick[(no + 1) % CHAIRNUM], 1, NULL);

                gotoxy(1, no * 4 + 1);
                printf("Mr. %c drops both chopsticks...\n", 'A' + no);
                fflush(stdout);
            }
            else {
                // ����޷�����ڶ������ӣ����µ�һ�����Ӳ������ӳ�
                ReleaseSemaphore(chopstick[no], 1, NULL);
                shortDelay();  // �����ӳٺ�����
            }
        }

        randomDelay();  // ģ���������˼��
    }

    gotoxy(1, no * 4 + 1);
    printf("Mr. %c is full...\n", 'A' + no);
    fflush(stdout);

    return 0;
}

// ����ӳٺ�����ģ��˼����Է�ʱ��
void randomDelay(void) {
    int stime = (rand() % 500) + 10;  // ����ӳ�10-510����
    Sleep(stime);
}

// �����ӳٺ���������Ƶ������
void shortDelay(void) {
    Sleep(10);  // �����ӳ�10����
}
