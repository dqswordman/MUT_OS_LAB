#include <iostream>
#include <windows.h>

int sharedVar1 = 0;
int sharedVar2 = 0;
bool running = true;  // 控制线程运行的标志
CRITICAL_SECTION lock;  // Windows的临界区，用于替代pthread中的mutex

// 线程函数声明
DWORD WINAPI threadFunctionA(LPVOID lpParam);
DWORD WINAPI threadFunctionB(LPVOID lpParam);
DWORD WINAPI threadFunctionC(LPVOID lpParam);

int main() {
    // 初始化临界区
    InitializeCriticalSection(&lock);

    // 创建线程句柄
    HANDLE hThread1 = CreateThread(NULL, 0, threadFunctionA, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, threadFunctionB, NULL, 0, NULL);
    HANDLE hThread3 = CreateThread(NULL, 0, threadFunctionC, NULL, 0, NULL);

    if (hThread1 == NULL || hThread2 == NULL || hThread3 == NULL) {
        std::cerr << "线程创建失败\n";
        return 1;
    }

    // 等待所有线程完成
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    WaitForSingleObject(hThread3, INFINITE);

    // 关闭线程句柄
    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(hThread3);

    // 删除临界区
    DeleteCriticalSection(&lock);

    return 0;
}

// 线程A：等待用户输入两个整数
DWORD WINAPI threadFunctionA(LPVOID lpParam) {
    while (true) {
        int localVar1, localVar2;
        std::cout << "请输入两个整数值：\n";
        std::cin >> localVar1 >> localVar2;

        EnterCriticalSection(&lock);
        sharedVar1 = localVar1;
        sharedVar2 = localVar2;
        LeaveCriticalSection(&lock);

        if (localVar1 > 100) {  // 当输入的第一个变量大于100时，停止所有线程
            running = false;  // 设置终止标志
            break;
        }
    }
    return 0;
}

// 线程B：计算并显示两个共享变量的和
DWORD WINAPI threadFunctionB(LPVOID lpParam) {
    while (running) {
        int localVar1, localVar2, sum;

        EnterCriticalSection(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        LeaveCriticalSection(&lock);

        sum = localVar1 + localVar2;
        std::cout << "和：" << sum << std::endl;

        Sleep(2000);  // 等待2秒

        if (!running) {
            break;
        }
    }
    return 0;
}

// 线程C：计算并显示两个共享变量的积
DWORD WINAPI threadFunctionC(LPVOID lpParam) {
    while (running) {
        int localVar1, localVar2, product;

        EnterCriticalSection(&lock);
        localVar1 = sharedVar1;
        localVar2 = sharedVar2;
        LeaveCriticalSection(&lock);

        product = localVar1 * localVar2;
        std::cout << "积：" << product << std::endl;

        Sleep(3000);  // 等待3秒

        if (!running) {
            break;
        }
    }
    return 0;
}
