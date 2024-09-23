#include <stdio.h>
#include <windows.h>

// 控制标志变量，确保按A->B->C顺序执行 每次间隔1s
int flagA = 1;  // 初始为线程A可以执行
int flagB = 0;  // 线程B等待
int flagC = 0;  // 线程C等待

CRITICAL_SECTION cs;  // 定义一个临界区对象

// 线程函数声明
DWORD WINAPI threadFunctionA(LPVOID arg);
DWORD WINAPI threadFunctionB(LPVOID arg);
DWORD WINAPI threadFunctionC(LPVOID arg);

int running = 1;  // 控制线程运行的标志

int main(void) {
    DWORD tid1, tid2, tid3;  // 线程 ID
    HANDLE th1, th2, th3;    // 线程句柄

    // 初始化临界区对象
    InitializeCriticalSection(&cs);

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

    // 删除临界区对象
    DeleteCriticalSection(&cs);

    return 0;
}

// 线程A：负责处理输入，并控制线程B执行
DWORD WINAPI threadFunctionA(LPVOID arg) {
    int userInput = 0;
    while (running) {
        Sleep(1000);// 每次开始前等1s

        EnterCriticalSection(&cs);  // 进入临界区
        if (flagA == 1) {  // 当flagA为1时，线程A执行
            printf("线程A正在运行\n");
            //scanf_s("%d", &userInput);  // 等待用户输入

            //if (userInput < 0) {
            //    running = 0;  // 设置终止标志
            //}

            // 线程A执行完毕，轮到线程B
            flagA = 0;
            flagB = 1;
        }
        LeaveCriticalSection(&cs);  // 离开临界区

        Sleep(100);  // 避免频繁占用CPU
    }
    return 0;
}

// 线程B：控制线程C的执行
DWORD WINAPI threadFunctionB(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // 进入临界区
        if (flagB == 1) {  // 当flagB为1时，线程B执行
            printf("线程B正在运行...\n");

            // 线程B执行完毕，轮到线程C
            flagB = 0;
            flagC = 1;
        }
        LeaveCriticalSection(&cs);  // 离开临界区

        Sleep(100);  // 避免频繁占用CPU
    }
    return 0;
}

// 线程C：完成后返回线程A
DWORD WINAPI threadFunctionC(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // 进入临界区
        if (flagC == 1) {  // 当flagC为1时，线程C执行
            printf("线程C正在运行...\n");

            // 线程C执行完毕，轮到线程A
            flagC = 0;
            flagA = 1;
        }
        LeaveCriticalSection(&cs);  // 离开临界区

        Sleep(100);  // 避免频繁占用CPU
    }
    return 0;
}
