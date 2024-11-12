/*
实验 4.1：使用 PCF8575 16位 GPIO 扩展器
PCF8575 介绍
PCF8575 是一个 16 位 I/O 扩展器，通过 I2C 总线连接。它由两个 8 位寄存器组成，分别连接到两个 8 位端口。在操作时，该设备无需命令字节，只需要两字节的数据来表示 16 位的 I/O 状态即可：

写模式：主设备（树莓派）向 PCF8575 发送两字节的 16 位数据，分别对应两个 8 位端口的输出状态。
读模式：主设备发送一个逻辑 1 表示要读取某一位，PCF8575 则返回包含 16 位 I/O 状态的数据。
*/

//4.1.1. 向 PCF8575 写数据的代码实现
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

//4.1.2. 从 PCF8575 读取数据的代码实现
//该代码展示了如何读取端口 P10 的数据，并对数据进行位操作。

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>

int running = true;
void gpio_stop(int sig);

int main() {
    int fd, i;
    uint16_t j;
    
    // 初始化 pigpio 库
    if (gpioInitialise() < 0) exit(-1);
    
    // 打开 I2C 设备，地址为 0x20
    if ((fd = i2cOpen(1, 0x20, 0)) < 0) exit(-1);
    
    // 设置终止信号处理
    signal(SIGINT, gpio_stop);
    printf("I2C PCF8575 16bit GPIO In/Out testing...\n");
    
    // 读取和处理数据
    while (running) {
        j = i2cReadWordData(fd, 0xff);  // 读取 16 位数据
        printf("%.2X\n", j);
        
        j >>= 8;  // 将数据右移 8 位
        for (i = 0; i < 16; i++) {
            i2cWriteByteData(fd, i & j, 0xff);  // 写入数据
            usleep(100000);
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




//4.1.3. 使用 Linux I2C 功能替代 Pigpio 库的代码实现
//该代码使用 Linux 的 I2C 功能，而非 pigpio 库。
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

int i2cInit(int busID);
void i2cError(int rw, int size);
void i2cWrite(int fd, const void *data, int size);
void i2cRead(int fd, void *data, int size);

int main() {
    int fd, i;
    uint8_t data[2];
    uint8_t flag;
    
    // 初始化 I2C 设备
    if ((fd = i2cInit(0x20)) == -1) exit(-1);
    printf("I2C PCF8575 16bit GPIO In/Out (using linux functions) testing...\n");
    
    while (1) {
        data[0] = data[1] = 0xff;
        i2cWrite(fd, data, 2);  // 写入两个字节的数据
        i2cRead(fd, data, 2);   // 读取两个字节的数据
        
        flag = data[1];  // 读取的数据放入 flag
        printf("%.2X%.2X\n", data[0], data[1]);
        
        for (i = 0; i < 16; i++) {
            data[0] = i & flag;
            i2cWrite(fd, data, 2);
            usleep(100000);
        }
        printf("."); fflush(stdout);
    }
    return 0;
}

int i2cInit(int busID) {
    char devname[] = "/dev/i2c-1";
    int fd;
    
    if ((fd = open(devname, O_RDWR)) < 0) {
        fprintf(stderr, "Fail to connect to the i2c device.\n");
        exit(-1);
    }
    if (ioctl(fd, I2C_SLAVE, busID) < 0) {
        fprintf(stderr, "Unable to communicate to the i2c device.\n");
        exit(-1);
    }
    return fd;
}

void i2cError(int rw, int size) {
    if (rw) {
        fprintf(stderr, "Error reading %d byte%s from i2c.\n", size, (size > 1) ? "s" : "");
        exit(-1);
    }
    fprintf(stderr, "Error writing %d byte%s to i2c.\n", size, (size > 1) ? "s" : "");
    exit(-1);
}

void i2cWrite(int fd, const void *data, int size) {
    if (write(fd, data, size) != size)
        i2cError(0, size);
}

void i2cRead(int fd, void *data, int size) {
    if (read(fd, data, size) != size)
        i2cError(1, size);
}


/*
实验 4.2.1：使用 ADS1115 模数转换器 (ADC)
实验概述
本实验通过树莓派与 ADS1115（一个 4 通道的模数转换器 IC）通信，学习如何操作需要命令字节的 I2C 设备。ADS1115 包含 4 个寄存器，用于配置和读取模拟输入的转换数据。

每次写操作需要命令字节加上 16 位数据，每次读操作也需要命令字节，ADS1115 将返回 16 位数据。
*/
//实验代码详解
//1.使用 pigpio 库的代码示例 以下代码展示了如何使用 pigpio 库与 ADS1115 进行通信：
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>

int running = true;
void gpio_stop(int sig);

int main() {
    int fd;
    uint16_t data;

    // 初始化 pigpio 库
    if (gpioInitialise() < 0) exit(-1);
    
    // 打开 I2C 设备，ADS1115 的地址为 0x48
    if ((fd = i2cOpen(1, 0x48, 0)) < 0) exit(-1);
    signal(SIGINT, gpio_stop);
    printf("I2C ADS1115 4ch ADC testing...\n");

    while (running) {
        // 写入配置寄存器，选择通道 0 进行读取
        i2cWriteWordData(fd, 1, 0x03c3);  // 0x03c3 配置为单次转换模式，读取通道 0

        // 等待转换完成
        while (((data = i2cReadWordData(fd, 1)) & 0x80) == 0) {
            usleep(100000);  // 延时等待转换完成
        }

        // 读取转换结果
        data = i2cReadWordData(fd, 0);

        // 字节顺序转换
        printf("Vout = %.4X\n", ((data >> 8) & 0xff) | ((data << 8) & 0xff00));
        fflush(stdout);
        usleep(250000);
    }

    i2cClose(fd);
    gpioTerminate();
    return 0;
}

void gpio_stop(int sig) {
    printf("Exiting..., please wait\n");
    running = false;
}


//4.2.2.使用 Linux I2C 功能的代码示例
//该代码展示了如何使用 Linux 的 I2C 功能来代替 pigpio 库，与 ADS1115 进行通信。
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

int i2cInit(int busID);
void i2cError(int rw, int size);
void i2cWrite(int fd, const void *data, int size);
void i2cRead(int fd, void *data, int size);

int main() {
    int fd;
    unsigned char data[4];

    // 初始化 I2C 设备
    if ((fd = i2cInit(0x48)) == -1) {
        return -1;
    }
    printf("I2C ADS1115 4-ch ADC testing (Using Linux I/O)...\n");

    while (1) {
        // 启动单次转换
        data[0] = 1;
        data[1] = 0xc3;
        data[2] = 0x03;
        i2cWrite(fd, data, 3);  // 写入配置寄存器

        data[0] = 0;
        // 等待转换完成
        while ((data[0] & 0x80) == 0) {
            i2cRead(fd, data, 2);
            usleep(100000);
        }

        // 读取结果
        data[0] = 0;
        i2cWrite(fd, data, 1);  // 从转换寄存器读取
        i2cRead(fd, data, 2);
        printf("Vout = %.2X%.2X\r", data[0], data[1]);
        fflush(stdout);
        usleep(250000);
    }
    return 0;
}

int i2cInit(int busID) {
    char devname[] = "/dev/i2c-1";
    int fd;

    if ((fd = open(devname, O_RDWR)) < 0) {
        fprintf(stderr, "Fail to connect to the i2c device.\n");
        exit(-1);
    }
    if (ioctl(fd, I2C_SLAVE, busID) < 0) {
        fprintf(stderr, "Unable to communicate to the i2c device.\n");
        exit(-1);
    }
    return fd;
}

/*
实验 4.3：使用 LDR 控制 LED 跑马灯速度
实验概述
在本实验中，我们将基于实验 3.8 的程序，通过 LDR（光敏电阻）控制 LED 跑马灯的运行速度。LDR 的阻值随着光照强度的变化而变化，因此可以用它来调节 LED 闪烁的速度。

实验原理
光敏电阻（LDR）会随着光照强度的变化而改变其阻值：

光线较强时，LDR 的阻值较低。
光线较弱时，LDR 的阻值较高。
利用这一特性，通过将 LDR 与电阻器连接形成分压电路，并将分压后的电压连接到树莓派的 ADS1115 模数转换器输入通道上，可以测量光强。根据测量到的模拟值，调整 LED 的闪烁速度。

实验步骤
连接 LDR 电路：

使用一个 10kΩ 电阻与 LDR 串联，将其连接到树莓派的 ADS1115 输入通道（例如通道 3）。
LDR 和电阻之间的连接点输出模拟电压，与光线强度成反比。
编写代码：在实验 3.8 的基础上修改代码，读取 LDR 的模拟电压，并根据读取值控制 LED 的闪烁速度。
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>

int running = true;
void gpio_stop(int sig);

int main() {
    int fd;
    uint16_t data;
    int delay;

    // 初始化 pigpio 库
    if (gpioInitialise() < 0) exit(-1);

    // 打开 I2C 设备，ADS1115 地址为 0x48
    if ((fd = i2cOpen(1, 0x48, 0)) < 0) exit(-1);

    signal(SIGINT, gpio_stop);
    printf("I2C ADS1115 - LED speed control with LDR\n");

    while (running) {
        // 配置 ADS1115 读取 LDR 通道（假设为通道 3）
        i2cWriteWordData(fd, 1, 0x03f3);  // 配置寄存器，通道 3，单次转换模式

        // 等待转换完成
        while (((data = i2cReadWordData(fd, 1)) & 0x80) == 0) {
            usleep(100000);
        }

        // 读取转换结果
        data = i2cReadWordData(fd, 0);
        // 交换高低字节顺序
        data = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);

        // 根据 LDR 的读取值设置 LED 闪烁延迟
        // 值越大（光线弱），延迟越大；值越小（光线强），延迟越小
        delay = 1000 + (data * 10);  // 简单映射延迟，您可以根据需要调整

        // 控制 LED 跑马灯的速度
        gpioWrite(LED_PIN, 1);  // 点亮 LED
        usleep(delay * 1000);   // 根据延迟值进行延时
        gpioWrite(LED_PIN, 0);  // 熄灭 LED
        usleep(delay * 1000);   // 再次延时

        printf("LED delay: %d ms based on LDR value: %d\n", delay, data);
        fflush(stdout);
    }

    i2cClose(fd);
    gpioTerminate();
    return 0;
}

void gpio_stop(int sig) {
    printf("Exiting..., please wait\n");
    running = false;
}

