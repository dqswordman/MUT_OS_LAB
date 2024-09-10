#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>

// 共享内存结构
struct shared_data {
    float val1;
    float val2;
};

// 终止值的常量定义
#define TERMINATE_VALUE 100.0

// 错误处理函数
void handle_error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int shmid;
    struct shared_data *shm;
    pid_t pidB, pidC;

    // 创建共享内存
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid < 0) handle_error("shmget failed");

    shm = (struct shared_data *)shmat(shmid, NULL, 0);
    if (shm == (struct shared_data *)-1) handle_error("shmat failed");

    // 初始化共享变量
    shm->val1 = 0.0;
    shm->val2 = 0.0;

    // 创建进程B
    if ((pidB = fork()) == 0) {
        // 进程B: 消费者，差值计算
        while (1) {
            sleep(2); // 等待2秒
            if (shm->val1 > TERMINATE_VALUE) break; // 检查终止条件
            printf("B: Difference is %.2f\n", shm->val2 - shm->val1);
        }
        exit(0);
    }
    // 创建进程C
    else if ((pidC = fork()) == 0) {
        // 进程C: 消费者，显示值
        while (1) {
            sleep(3); // 等待3秒
            if (shm->val1 > TERMINATE_VALUE) break; // 检查终止条件
            printf("C: Values are %.2f and %.2f\n", shm->val1, shm->val2);
        }
        exit(0);
    }
    // 父进程A: 生产者，获取用户输入
    else {
        while (1) {
            printf("Enter two floats (first > %.1f to quit): ", TERMINATE_VALUE);
            if (scanf("%f %f", &shm->val1, &shm->val2) != 2) {
                printf("Invalid input. Please enter two float numbers.\n");
                //while (getchar() != '\n'); // 清空输入缓冲区
                continue;
            }
            if (shm->val1 > TERMINATE_VALUE) break; // 检查终止条件
        }
        // 等待进程B和C终止
        wait(NULL);
        wait(NULL);
        // 解除共享内存映射并删除
        if (shmdt(shm) == -1) handle_error("shmdt failed");
        if (shmctl(shmid, IPC_RMID, NULL) == -1) handle_error("shmctl failed");

        exit(0);
    }
}

