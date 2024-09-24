#include <stdio.h>
#include <windows.h>

// 共享变量
float sharedVar1 = 0.0;
float sharedVar2 = 0.0;
int running = 1;  // 控制线程运行的标志

CRITICAL_SECTION cs;  // 定义一个临界区对象


// 线程函数声明
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

// 线程A：等待用户输入两个浮点值
DWORD WINAPI threadFunctionA(LPVOID arg) {
    while (running) {
        EnterCriticalSection(&cs);  // 进入临界区
        printf("请输入两个浮点值：\n");
        scanf_s("%f %f", &sharedVar1, &sharedVar2);  // 简化输入处理

        if (sharedVar1 > 100.0) {
            running = 0;  // 设置终止标志
        }
        LeaveCriticalSection(&cs);  // 离开临界区

        Sleep(100);  // 避免频繁锁住临界区
    }
    return 0;
}

// 线程B：计算并显示两个共享变量的差
DWORD WINAPI threadFunctionB(LPVOID arg) {
    while (running) {
        printf("差值：%f\n", sharedVar2 - sharedVar1);
        Sleep(2000);  // 每2秒输出一次差值
    }
    return 0;
}

// 线程C：显示两个共享变量的值
DWORD WINAPI threadFunctionC(LPVOID arg) {
    while (running) {
        printf("当前值：Var1 = %f, Var2 = %f\n", sharedVar1, sharedVar2);
        Sleep(3000);  // 每3秒显示一次当前值
    }
    return 0;
}
