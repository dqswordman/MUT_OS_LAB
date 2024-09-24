#include <iostream>
#include <windows.h>

float sharedVar1 = 0.0f;
float sharedVar2 = 0.0f;
bool running = true;  // �����߳����еı�־
CRITICAL_SECTION lock;  // Windows���ٽ������������pthread�е�mutex

// �̺߳�������
DWORD WINAPI threadFunctionA(LPVOID lpParam);
DWORD WINAPI threadFunctionB(LPVOID lpParam);
DWORD WINAPI threadFunctionC(LPVOID lpParam);

int main() {
    // ��ʼ���ٽ���
    InitializeCriticalSection(&lock);

    // �����߳̾��
    HANDLE hThread1 = CreateThread(NULL, 0, threadFunctionA, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, threadFunctionB, NULL, 0, NULL);
    HANDLE hThread3 = CreateThread(NULL, 0, threadFunctionC, NULL, 0, NULL);

    if (hThread1 == NULL || hThread2 == NULL || hThread3 == NULL) {
        std::cerr << "�̴߳���ʧ��\n";
        return 1;
    }

    // �ȴ������߳����
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    WaitForSingleObject(hThread3, INFINITE);

    // �ر��߳̾��
    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(hThread3);

    // ɾ���ٽ���
    DeleteCriticalSection(&lock);

    return 0;
}

// �߳�A���ȴ��û���������������
DWORD WINAPI threadFunctionA(LPVOID lpParam) {
    while (true) {
        float localVar1, localVar2;
        std::cout << "��������������ֵ��\n";
        std::cin >> localVar1 >> localVar2;

        EnterCriticalSection(&lock);
        sharedVar1 = localVar1;
        sharedVar2 = localVar2;
        LeaveCriticalSection(&lock);

        if (localVar1 > 100.0f) {  // ������ĵ�һ����������100ʱ��ֹͣ�����߳�
            running = false;  // ������ֹ��־
            break;
        }
    }
    return 0;
}

// �߳�B�����㲢��ʾ������������ĺ�
DWORD WINAPI threadFunctionB(LPVOID lpParam) {
    while (running) {
        float localVar1, localVar2, sum;

        EnterCriticalSection(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        LeaveCriticalSection(&lock);

        std::cout << "��������" << localVar1 << " , " << localVar2 << std::endl;

        Sleep(2000);  // �ȴ�2��

        if (!running) {
            break;
        }
    }
    return 0;
}

// �߳�C�����㲢��ʾ������������Ļ�
DWORD WINAPI threadFunctionC(LPVOID lpParam) {
    while (running) {
        float localVar1, localVar2, product;

        EnterCriticalSection(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        LeaveCriticalSection(&lock);

        //product = localVar1 * localVar2;
        std::cout << "��ֵ :" << localVar2 - localVar1 << std::endl;

        Sleep(3000);  // �ȴ�3��

        if (!running) {
            break;
        }
    }
    return 0;
}
