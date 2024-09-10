#include <stdio.h>
#include <windows.h>

// 声明三个独立的线程函数
DWORD WINAPI threadFunctionA(LPVOID arg);
DWORD WINAPI threadFunctionB(LPVOID arg);
DWORD WINAPI threadFunctionC(LPVOID arg);

int main(void) {
    DWORD tid1, tid2, tid3;  // 线程 ID
    HANDLE th1, th2, th3;    // 线程句柄

    // 创建 3 个线程
    th1 = CreateThread(
        NULL,                // 默认安全属性
        0,                   // 默认栈大小
        threadFunctionA,     // 线程函数
        NULL,                // 线程函数参数
        0,                   // 默认创建标志
        &tid1                // 返回的线程 ID
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

    // 等待所有线程完成
    if (th1 != NULL && th2 != NULL && th3 != NULL) {
        WaitForSingleObject(th1, INFINITE);
        WaitForSingleObject(th2, INFINITE);
        WaitForSingleObject(th3, INFINITE);

        // 关闭线程句柄
        CloseHandle(th1);
        CloseHandle(th2);
        CloseHandle(th3);
    }

    return 0;
}

DWORD WINAPI threadFunctionA(LPVOID arg) {
    printf("Running from thread A\n");
    Sleep(1000);  // 模拟线程运行延迟
    return 0;
}

DWORD WINAPI threadFunctionB(LPVOID arg) {
    printf("Running from thread B\n");
    Sleep(1000);  // 模拟线程运行延迟
    return 0;
}

DWORD WINAPI threadFunctionC(LPVOID arg) {
    printf("Running from thread C\n");
    Sleep(1000);  // 模拟线程运行延迟
    return 0;
}
