#include <stdio.h>
#include <windows.h>

// ���Ʊ�־������ȷ����A->B->C˳��ִ�� ÿ�μ��1s
int flagA = 1;  // ��ʼΪ�߳�A����ִ��
int flagB = 0;  // �߳�B�ȴ�
int flagC = 0;  // �߳�C�ȴ�

CRITICAL_SECTION cs;  // ����һ���ٽ�������

// �̺߳�������
DWORD WINAPI threadFunctionA(LPVOID arg);
DWORD WINAPI threadFunctionB(LPVOID arg);
DWORD WINAPI threadFunctionC(LPVOID arg);

int running = 1;  // �����߳����еı�־

int main(void) {
    DWORD tid1, tid2, tid3;  // �߳� ID
    HANDLE th1, th2, th3;    // �߳̾��

    // ��ʼ���ٽ�������
    InitializeCriticalSection(&cs);

    // ���� 3 ���߳�
    th1 = CreateThread(
        NULL,                // Ĭ�ϰ�ȫ����
        0,                   // Ĭ��ջ��С
        threadFunctionA,     // �̺߳���
        NULL,                // �̺߳�������
        0,                   // Ĭ�ϴ�����־
        &tid1                // ���ص��߳� ID
    );

    th2 = CreateThread(
        NULL,
        0,
        threadFunctionB,
        NULL,
        0,
        &tid2
    );

    th3 = CreateThread(
        NULL,
        0,
        threadFunctionC,
        NULL,
        0,
        &tid3
    );

    // �ȴ������߳����
    if (th1 != NULL && th2 != NULL && th3 != NULL) {
        WaitForSingleObject(th1, INFINITE);
        WaitForSingleObject(th2, INFINITE);
        WaitForSingleObject(th3, INFINITE);

        // �ر��߳̾��
        CloseHandle(th1);
        CloseHandle(th2);
        CloseHandle(th3);
    }

    // ɾ���ٽ�������
    DeleteCriticalSection(&cs);

    return 0;
}

// �߳�A�����������룬�������߳�Bִ��
DWORD WINAPI threadFunctionA(LPVOID arg) {
    int userInput = 0;
    while (running) {
        Sleep(1000);// ÿ�ο�ʼǰ��1s

        EnterCriticalSection(&cs);  // �����ٽ���
        if (flagA == 1) {  // ��flagAΪ1ʱ���߳�Aִ��
            printf("�߳�A��������\n");
            //scanf_s("%d", &userInput);  // �ȴ��û�����

            //if (userInput < 0) {
            //    running = 0;  // ������ֹ��־
            //}

            // �߳�Aִ����ϣ��ֵ��߳�B
            flagA = 0;
            flagB = 1;
        }
        LeaveCriticalSection(&cs);  // �뿪�ٽ���

        Sleep(100);  // ����Ƶ��ռ��CPU
    }
    return 0;
}

// �߳�B�������߳�C��ִ��
DWORD WINAPI threadFunctionB(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // �����ٽ���
        if (flagB == 1) {  // ��flagBΪ1ʱ���߳�Bִ��
            printf("�߳�B��������...\n");

            // �߳�Bִ����ϣ��ֵ��߳�C
            flagB = 0;
            flagC = 1;
        }
        LeaveCriticalSection(&cs);  // �뿪�ٽ���

        Sleep(100);  // ����Ƶ��ռ��CPU
    }
    return 0;
}

// �߳�C����ɺ󷵻��߳�A
DWORD WINAPI threadFunctionC(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // �����ٽ���
        if (flagC == 1) {  // ��flagCΪ1ʱ���߳�Cִ��
            printf("�߳�C��������...\n");

            // �߳�Cִ����ϣ��ֵ��߳�A
            flagC = 0;
            flagA = 1;
        }
        LeaveCriticalSection(&cs);  // �뿪�ٽ���

        Sleep(100);  // ����Ƶ��ռ��CPU
    }
    return 0;
}
