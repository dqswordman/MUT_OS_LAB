//4.1 
//1. 向 PCF8575 写数据的代码实现
//这段代码展示了如何使用 pigpio 库将输出写入 PCF8575。
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>

int running = true;
void gpio_stop(int sig);

int main() {
    int fd, i;
    
    // 初始化 pigpio 库
    if (gpioInitialise() < 0) exit(-1);
    
    // 打开 I2C 设备，地址为 0x20
    if ((fd = i2cOpen(1, 0x20, 0)) < 0) exit(-1);
    
    // 设置终止信号处理
    signal(SIGINT, gpio_stop);
    printf("I2C PCF8575 16bit GPIO testing...\n");
    
    // 循环发送数据
    while (running) {
        for (i = 0; i < 16; i++) {
            i2cWriteByteData(fd, i, 0xff);  // 将 0xff 写入设备
            usleep(100000);  // 延时 100 毫秒
        }
        printf("."); fflush(stdout);
    }
    
    // 关闭设备和库
    i2cClose(fd);
    gpioTerminate();
    return 0;
}

void gpio_stop(int sig) {
    printf("Exiting..., please wait\n");
    running = false;
}




