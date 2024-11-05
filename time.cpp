#include <wiringPi.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>

#define TIMEOUT 10000
int trig = 19;
int echo = 20;
int ledPin = 17; // GPIO引脚号
int running = 1;

void initGPIO();
int ultraSonic();

int main() {
    int delay;
    wiringPiSetupGpio(); // 初始化GPIO用于LED
    pinMode(ledPin, OUTPUT); // 将LED引脚设置为输出

    initGPIO(); // 初始化超声波传感器的GPIO

    while (running) {
        delay = ultraSonic(); // 获取超声波测量的时间差
        if (delay >= 0) {
            // 计算距离（以厘米为单位）
            int distance = delay * 0.0343 / 2;
            
            // 根据距离设置LED闪烁速度，距离越近，延迟越小
            int ledDelay = distance * 1000; // 将距离转换为延迟时间，单位微秒
            if (ledDelay < 10000) ledDelay = 10000; // 设置最小延迟时间为10毫秒
            if (ledDelay > 1000000) ledDelay = 1000000; // 设置最大延迟时间为1秒

            digitalWrite(ledPin, HIGH); // 点亮LED
            usleep(ledDelay);           // 根据计算的延迟时间等待
            digitalWrite(ledPin, LOW);  // 熄灭LED
            usleep(ledDelay);           // 再次等待
        }
    }
    gpioTerminate();
    return 0;
}

void initGPIO() {
    if (gpioInitialise() < 0) exit(1);
    gpioSetMode(trig, PI_OUTPUT);
    gpioWrite(trig, 0);
    gpioSetMode(echo, PI_INPUT);
    gpioSetPullUpDown(echo, PI_PUD_OFF);
    sleep(2);
}

int ultraSonic() {
    int clock1, clock2, timeout;

    gpioWrite(trig, 1);
    usleep(10);
    gpioWrite(trig, 0);

    for (timeout = 0; (timeout < TIMEOUT) && (gpioRead(echo) == 0); timeout++)
        usleep(10);
    clock1 = clock();
    if (timeout >= TIMEOUT) return -1;

    for (timeout = 0; (timeout < TIMEOUT) && (gpioRead(echo) == 1); timeout++)
        usleep(10);
    clock2 = clock();
    if (timeout >= TIMEOUT) return -1;

    return clock2 - clock1;
}
