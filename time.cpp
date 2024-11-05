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




#include <pigpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define TIMEOUT 10000
int trig = 19;
int echo = 20;
int ledPins[4] = {4, 5, 6, 12}; // LED的GPIO引脚
int running = 1;

void initGPIO();
int ultraSonic();
void gpio_stop(int sig);

// 信号处理函数
void gpio_stop(int sig) {
    printf("User pressing CTRL-C\n");
    gpioTerminate();
    exit(0);
}

int main() {
    if (gpioInitialise() < 0) {
        printf("GPIO initialization failed!\n");
        return 1;
    }

    // 初始化LED引脚
    for (int i = 0; i < 4; i++) {
        gpioSetMode(ledPins[i], PI_OUTPUT);
    }
    initGPIO(); // 初始化超声波传感器的GPIO

    signal(SIGINT, gpio_stop); // 处理CTRL-C中断信号

    while (running) {
        int delay = ultraSonic(); // 获取超声波测量的时间差
        if (delay >= 0) {
            // 计算距离（以厘米为单位）
            int distance = delay * 0.0343 / 2;

            // 根据距离设置LED闪烁速度，距离越近，延迟越小
            int ledDelay = distance * 1000; // 将距离转换为延迟时间，单位微秒
            if (ledDelay < 10000) ledDelay = 10000; // 设置最小延迟时间为10毫秒
            if (ledDelay > 1000000) ledDelay = 1000000; // 设置最大延迟时间为1秒

            // 控制LED以运行灯的形式闪烁
            for (int i = 0; i < 4; i++) {
                gpioWrite(ledPins[i], 1); // 点亮当前LED
                usleep(ledDelay);         // 延迟
                gpioWrite(ledPins[i], 0); // 熄灭当前LED
            }
        }
    }
    gpioTerminate();
    return 0;
}

// 初始化超声波传感器的GPIO
void initGPIO() {
    if (gpioInitialise() < 0) exit(1);
    gpioSetMode(trig, PI_OUTPUT);
    gpioWrite(trig, 0);
    gpioSetMode(echo, PI_INPUT);
    gpioSetPullUpDown(echo, PI_PUD_OFF);
    sleep(2);
}

// 超声波测量函数
int ultraSonic() {
    int clock1, clock2, timeout;

    gpioWrite(trig, 1);
    usleep(10);
    gpioWrite(trig, 0);

    for (timeout = 0; (timeout < TIMEOUT) && (gpioRead(echo) == 0); timeout++)
        usleep(10);
    clock1 = gpioTick(); // 记录当前时间，以微秒为单位
    if (timeout >= TIMEOUT) return -1;

    for (timeout = 0; (timeout < TIMEOUT) && (gpioRead(echo) == 1); timeout++)
        usleep(10);
    clock2 = gpioTick(); // 记录当前时间，以微秒为单位
    if (timeout >= TIMEOUT) return -1;

    return clock2 - clock1;
}








#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include <signal.h>
#include <stdlib.h>

int PWM_pin = 18; // 使用GPIO18引脚作为PWM输出

void gpio_stop(int sig); // 停止GPIO的信号处理函数

int main() {
    int i;
    printf("LED PWM (0%%-100%% duty cycle)\n");
    
    if (gpioInitialise() < 0) { // 初始化pigpio库
        return -1; // 如果初始化失败，返回-1
    }

    signal(SIGINT, gpio_stop); // 注册信号处理函数，用于处理CTRL-C中断

    while (1) {
        // 逐步增加占空比，从0%到100%
        for (i = 0; i < 100; i++) {
            gpioHardwarePWM(PWM_pin, 50, i * 10000); // 设置PWM频率为50 Hz，占空比逐步增加
            usleep(10000); // 延迟10毫秒
        }
        // 逐步减少占空比，从100%到0%
        for (i = 99; i > 0; i--) {
            gpioHardwarePWM(PWM_pin, 50, i * 10000); // 设置PWM频率为50 Hz，占空比逐步减少
            usleep(10000); // 延迟10毫秒
        }
    }

    return 0;
}

// 信号处理函数，用于终止程序时释放GPIO资源
void gpio_stop(int sig) {
    printf("User pressing CTRL-C\n");
    gpioTerminate(); // 释放pigpio库资源
    exit(0); // 退出程序
}

