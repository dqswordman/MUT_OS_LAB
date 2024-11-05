#include <pigpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int keyGPIO_row[4] = {21, 22, 23, 24};
int keyGPIO_col[3] = {25, 26, 27};
int keyMap[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
int ledPins[4] = {4, 5, 6, 12}; // LED的GPIO引脚

void initGPIO();
int getch();
void gpio_stop(int sig);

// 信号处理函数
void gpio_stop(int sig) {
    printf("User pressing CTRL-C\n");
    gpioTerminate();
    exit(0);
}

int main() {
    int ch, delay;
    initGPIO();
    
    for (int i = 0; i < 4; i++) {
        gpioSetMode(ledPins[i], PI_OUTPUT); // 初始化LED引脚
    }

    signal(SIGINT, gpio_stop); // 处理CTRL-C中断信号

    while (1) {
        printf("Please press a key on keypad:\n");
        fflush(stdout);
        ch = getch(); // 获取键盘输入

        if (ch > 0) {
            printf("Key = %c \n", ch);
            fflush(stdout);

            if (ch == '0' || ch == '#' || ch == '*') {
                break; // 如果按下0、#或*，程序终止
            }

            // 将ASCII字符转换为整数
            int speed = ch - '0';
            if (speed >= 1 && speed <= 9) {
                // 根据速度调整LED运行延迟时间
                delay = 1000000 / speed; // 设置延迟时间，速度越快延迟越小

                // 控制LED以运行灯的形式闪烁
                for (int i = 0; i < 4; i++) {
                    gpioWrite(ledPins[i], 1); // 点亮当前LED
                    usleep(delay);            // 延迟
                    gpioWrite(ledPins[i], 0); // 熄灭当前LED
                }
            }
        } else {
            usleep(10000); // 等待10毫秒
        }
    }
    gpioTerminate();
    return 0;
}

// 初始化GPIO
void initGPIO() {
    int i;
    if (gpioInitialise() < 0) exit(1);
    for (i = 0; i < 4; i++) {
        gpioSetMode(keyGPIO_row[i], PI_OUTPUT);
        gpioWrite(keyGPIO_row[i], 1);
    }
    for (i = 0; i < 3; i++) {
        gpioSetMode(keyGPIO_col[i], PI_INPUT);
        gpioSetPullUpDown(keyGPIO_col[i], PI_PUD_UP);
    }
}

// 获取键盘输入
int getch() {
    int row, col;
    for (row = 0; row < 4; row++) {
        gpioWrite(keyGPIO_row[row], 0);
        for (col = 0; col < 3; col++) {
            if (gpioRead(keyGPIO_col[col]) == 0) break;
        }
        if (col < 3) break; // 检测到按键
        gpioWrite(keyGPIO_row[row], 1);
    }
    if (row < 4) {
        while (gpioRead(keyGPIO_col[col]) == 0) usleep(10000);
        gpioWrite(keyGPIO_row[row], 1);
        return keyMap[row][col];
    } else {
        return 0;
    }
}
