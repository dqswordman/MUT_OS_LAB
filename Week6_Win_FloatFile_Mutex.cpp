#include <stdio.h>
#include <windows.h>

// �������
float sharedVar1 = 0.0;
float sharedVar2 = 0.0;
int running = 1;  // �����߳����еı�־

CRITICAL_SECTION cs;  // ����һ���ٽ�������


// �̺߳�������
DWORD WINAPI threadFunctionA(LPVOID arg);
DWORD WINAPI threadFunctionB(LPVOID arg);
DWORD WINAPI threadFunctionC(LPVOID arg);

int main(void) {
    DWORD tid1, tid2, tid3;  // �߳� ID
    HANDLE th1, th2, th3;    // �߳̾��

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

    return 0;
}

// �߳�A���ȴ��û�������������ֵ
DWORD WINAPI threadFunctionA(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // �����ٽ���
        printf("��������������ֵ��\n");
        scanf_s("%f %f", &sharedVar1, &sharedVar2);  // �����봦��

        if (sharedVar1 > 100.0) {
            running = 0;  // ������ֹ��־
        }
        LeaveCriticalSection(&cs);  // �뿪�ٽ���

        Sleep(100);  // ����Ƶ����ס�ٽ���
    }
    return 0;
}

// �߳�B�����㲢��ʾ������������Ĳ�
DWORD WINAPI threadFunctionB(LPVOID arg) {
    while (running) {
        printf("��ֵ��%f\n", sharedVar2 - sharedVar1);
        Sleep(2000);  // ÿ2�����һ�β�ֵ
    }
    return 0;
}

// �߳�C����ʾ�������������ֵ
DWORD WINAPI threadFunctionC(LPVOID arg) {
    while (running) {
        printf("��ǰֵ��Var1 = %f, Var2 = %f\n", sharedVar1, sharedVar2);
        Sleep(3000);  // ÿ3����ʾһ�ε�ǰֵ
    }
    return 0;
}
