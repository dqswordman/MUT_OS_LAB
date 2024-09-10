#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>

// 共享内存结构
struct shared_data {
    int val1;
    int val2;
};

int main() {
    int shmid;
    struct shared_data *shm;
    pid_t pidB, pidC;

    // 创建共享内存
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("共享内存创建失败");
        exit(1);
    }

    // 连接到共享内存
    shm = (struct shared_data *) shmat(shmid, NULL, 0);
    if (shm == (struct shared_data *) -1) {
        perror("共享内存连接失败");
        exit(1);
    }

    // 初始化共享变量
    shm->val1 = 0;
    shm->val2 = 0;

    // 创建进程B
    if ((pidB = fork()) == 0) {
        // 进程B: 消费者，加法
        while (1) {
            sleep(2); // 等待2秒
            if (shm->val1 < 0) break; // 检查终止条件
            printf("B: Sum is %d\n", shm->val1 + shm->val2);
        }
        shmdt(shm); // 解除共享内存映射
        exit(0);
    } else if (pidB < 0) {
        perror("进程B创建失败");
        exit(1);
    }

    // 创建进程C
    if ((pidC = fork()) == 0) {
        // 进程C: 消费者，乘法
        while (1) {
            sleep(3); // 等待3秒
            if (shm->val1 < 0) break; // 检查终止条件
            printf("C: Product is %d\n", shm->val1 * shm->val2);
        }
        shmdt(shm); // 解除共享内存映射
        exit(0);
    } else if (pidC < 0) {
        perror("进程C创建失败");
        exit(1);
    }

    // 进程A: 生产者，获取用户输入
    while (1) {
        printf("Enter two integers (negative first to quit): ");
        scanf("%d %d", &shm->val1, &shm->val2);
        if (shm->val1 < 0) break; // 检查终止条件
    }

    // 等待进程B和C终止
    wait(NULL);
    wait(NULL);

    // 清理共享内存
    shmdt(shm); // 解除共享内存映射
    shmctl(shmid, IPC_RMID, NULL); // 删除共享内存

    return 0;
}
