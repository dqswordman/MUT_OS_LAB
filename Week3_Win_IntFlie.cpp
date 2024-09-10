#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

// 定义共享数据的结构体
struct SharedData {
    int num1;
    int num2;
};

// 生产者进程的函数
void Producer(struct SharedData* sharedData) {
    while (1) {
        printf("请输入两个整数值（以空格分隔）：");
        scanf("%d %d", &sharedData->num1, &sharedData->num2);
        
        if (sharedData->num1 < 0) {  // 如果第一个值是负数，则生产者终止
            printf("生产者进程终止...\n");
            break;
        }
    }
}

// 消费者进程 B 的函数
void ConsumerAdd(struct SharedData* sharedData) {
    while (1) {
        Sleep(2000);  // 等待 2 秒钟
        
        if (sharedData->num1 < 0) {  // 检查第一个值是否为负数
            printf("消费者进程 B 终止...\n");
            break;
        }
        
        int sum = sharedData->num1 + sharedData->num2;
        printf("消费者进程 B: %d + %d = %d\n", sharedData->num1, sharedData->num2, sum);
    }
}

// 消费者进程 C 的函数
void ConsumerMultiply(struct SharedData* sharedData) {
    while (1) {
        Sleep(3000);  // 等待 3 秒钟
        
        if (sharedData->num1 < 0) {  // 检查第一个值是否为负数
            printf("消费者进程 C 终止...\n");
            break;
        }
        
        int product = sharedData->num1 * sharedData->num2;
        printf("消费者进程 C: %d * %d = %d\n", sharedData->num1, sharedData->num2, product);
    }
}

int main(int argc, char *argv[]) {
    HANDLE hMapFile;
    struct SharedData* sharedData;
    TCHAR shmName[] = _T("Global\\SharedMemoryExample");
    int shmsize = sizeof(struct SharedData);

    if (argc < 2) {
        // 创建内存映射文件
        hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            shmsize,
            shmName
        );

        if (hMapFile == NULL) {
            printf("无法创建文件映射对象 (%d).\n", GetLastError());
            return -1;
        }

        sharedData = (struct SharedData*) MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            shmsize
        );

        if (sharedData == NULL) {
            printf("无法映射文件视图 (%d).\n", GetLastError());
            CloseHandle(hMapFile);
            return -1;
        }

        // 初始化共享数据
        sharedData->num1 = 0;
        sharedData->num2 = 0;

        STARTUPINFO si[2];
        PROCESS_INFORMATION pi[2];

        // 创建消费者进程 B
        ZeroMemory(&si[0], sizeof(si[0]));
        si[0].cb = sizeof(si[0]);
        ZeroMemory(&pi[0], sizeof(pi[0]));
        TCHAR consumerBName[] = _T("Week3_win_IntFlie.exe 1");

        if (!CreateProcess(NULL, consumerBName, NULL, NULL, FALSE, 0, NULL, NULL, &si[0], &pi[0])) {
            fprintf(stderr, "创建进程 B 失败。\n");
            return -1;
        }

        // 创建消费者进程 C
        ZeroMemory(&si[1], sizeof(si[1]));
        si[1].cb = sizeof(si[1]);
        ZeroMemory(&pi[1], sizeof(pi[1]));
        TCHAR consumerCName[] = _T("Week3_win_IntFlie.exe 2");

        if (!CreateProcess(NULL, consumerCName, NULL, NULL, FALSE, 0, NULL, NULL, &si[1], &pi[1])) {
            fprintf(stderr, "创建进程 C 失败。\n");
            return -1;
        }

        // 生产者进程 A
        Producer(sharedData);

        // 等待消费者进程结束
        WaitForSingleObject(pi[0].hProcess, INFINITE);
        WaitForSingleObject(pi[1].hProcess, INFINITE);

        // 清理句柄
        UnmapViewOfFile(sharedData);
        CloseHandle(hMapFile);
        CloseHandle(pi[0].hProcess);
        CloseHandle(pi[0].hThread);
        CloseHandle(pi[1].hProcess);
        CloseHandle(pi[1].hThread);

        printf("所有进程已终止。\n");
    } else {
        // 打开现有的内存映射文件
        hMapFile = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            shmName
        );

        if (hMapFile == NULL) {
            printf("无法打开文件映射对象 (%d).\n", GetLastError());
            return -1;
        }

        sharedData = (struct SharedData*) MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            shmsize
        );

        if (sharedData == NULL) {
            printf("无法映射文件视图 (%d).\n", GetLastError());
            CloseHandle(hMapFile);
            return -1;
        }

        // 子进程根据命令行参数选择运行对应的消费者逻辑
        if (atoi(argv[1]) == 1) {
            ConsumerAdd(sharedData);
        } else if (atoi(argv[1]) == 2) {
            ConsumerMultiply(sharedData);
        }

        UnmapViewOfFile(sharedData);
        CloseHandle(hMapFile);
    }
    return 0;
}
