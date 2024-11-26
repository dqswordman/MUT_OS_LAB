//1.0
#include <pigpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <algorithm>

#pragma pack(1)

#define FRONT   0
#define BACK    1
#define LEFT    2
#define RIGHT   3

#define PORT 8888
#define PWM_RAMP_RATE 5
#define ULTRASONIC_BUFFER_SIZE 5
#define USDETECT 6
#define USMIN 3

#define PWMDELAY        10000
#define PWMDELAY2       100

volatile int pwmA = 0, pwmB = 0;
volatile int pwmC = 50, pwmD = 50;
uint8_t running = true;

typedef struct {
    uint8_t distance[4];
    int8_t speed;
    int8_t direction;
    uint8_t autoMode;
    uint8_t sensorSearch;
    uint16_t CRC;
} carStatusType;
carStatusType carStatus = {{0}, 0, 0, 0, 0, 0};

typedef struct {
    int8_t speed;
    int8_t direction;
    int8_t calibrate;
    uint8_t autoMode;
    uint8_t sensorSearch;
    uint8_t dummy;
    uint16_t CRC;
} carControlType;
carControlType carControl = {0, 0, 0, 0, 0};

int sensorTrig[4] = {20, 22, 24, 26};
int sensorEcho[4] = {21, 23, 25, 27};

int distanceBuffer[4][ULTRASONIC_BUFFER_SIZE] = {0};

void gpioStop(int sig);
void initGPIO();
void updatePWM(int *currentPWM, int targetPWM, int rampRate);
void addToBuffer(int id, int value);
int getMedian(int id);
void *ultraSonic(void *param);
void *networkControl(void *param);
void *command(void *param);
void *PWMControl(void *param);

int main() {
    pthread_t tid[4];
    pthread_attr_t attr[4];

    initGPIO();
    signal(SIGINT, gpioStop);

    pthread_attr_init(&attr[0]);
    pthread_create(&tid[0], &attr[0], ultraSonic, NULL);

    pthread_attr_init(&attr[1]);
    pthread_create(&tid[1], &attr[1], networkControl, NULL);

    pthread_attr_init(&attr[2]);
    pthread_create(&tid[2], &attr[2], command, NULL);

    pthread_attr_init(&attr[3]);
    pthread_create(&tid[3], &attr[3], PWMControl, NULL);

    while (running) {
        printf("Distances: F[%d] B[%d] L[%d] R[%d] || Speed: [%d] Direction: [%d]\r",
               carStatus.distance[FRONT], carStatus.distance[BACK],
               carStatus.distance[LEFT], carStatus.distance[RIGHT],
               carStatus.speed, carStatus.direction);
        fflush(stdout);
        usleep(100000);
    }

    printf("Waiting for threads to stop...\n");
    for (int i = 0; i < 4; i++) {
        pthread_join(tid[i], NULL);
    }
    gpioTerminate();
    return 0;
}

void gpioStop(int sig) {
    printf("CTRL-C pressed.\n");
    running = false;
}

void initGPIO() {
    if (gpioInitialise() < 0) exit(-1);

    for (int i = 0; i < 4; i++) {
        gpioSetMode(sensorTrig[i], PI_OUTPUT);
        gpioSetMode(sensorEcho[i], PI_INPUT);
        gpioSetPullUpDown(sensorEcho[i], PI_PUD_OFF);
    }
}

void updatePWM(int *currentPWM, int targetPWM, int rampRate) {
    if (*currentPWM < targetPWM) {
        *currentPWM += rampRate;
        if (*currentPWM > targetPWM) *currentPWM = targetPWM;
    } else if (*currentPWM > targetPWM) {
        *currentPWM -= rampRate;
        if (*currentPWM < targetPWM) *currentPWM = targetPWM;
    }
}

void addToBuffer(int id, int value) {
    for (int i = ULTRASONIC_BUFFER_SIZE - 1; i > 0; i--) {
        distanceBuffer[id][i] = distanceBuffer[id][i - 1];
    }
    distanceBuffer[id][0] = value;
}

int getMedian(int id) {
    int sorted[ULTRASONIC_BUFFER_SIZE];
    memcpy(sorted, distanceBuffer[id], sizeof(sorted));
    std::sort(sorted, sorted + ULTRASONIC_BUFFER_SIZE);
    return sorted[ULTRASONIC_BUFFER_SIZE / 2];
}

void *ultraSonic(void *param) {
    int clock1, clock2, dclock;
    while (running) {
        for (int id = 0; id < 4; id++) {
            gpioWrite(sensorTrig[id], 1);
            usleep(10);
            gpioWrite(sensorTrig[id], 0);
            while (gpioRead(sensorEcho[id]) == 0) clock1 = clock();
            while (gpioRead(sensorEcho[id]) == 1) clock2 = clock();
            dclock = (clock2 - clock1) / 100;
            addToBuffer(id, dclock);
            carStatus.distance[id] = getMedian(id);
        }
        usleep(100000);
    }
    pthread_exit(NULL);
}

void *networkControl(void *param) {
    struct sockaddr_in s_me, s_other;
    int s, rLen, s_len = sizeof(s_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("Cannot start UDP server\n");
        running = false;
        pthread_exit(NULL);
    }

    memset((char *)&s_me, 0, sizeof(s_me));
    s_me.sin_family = AF_INET;
    s_me.sin_port = htons(PORT);
    s_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&s_me, sizeof(s_me)) == -1) {
        printf("Cannot bind UDP port\n");
        close(s);
        running = false;
        pthread_exit(NULL);
    }

    while (running) {
        if ((rLen = recvfrom(s, &carControl, sizeof(carControlType), 0,
                             (struct sockaddr *)&s_other, (socklen_t *)&s_len)) < 0) {
            continue;
        }
        sendto(s, &carStatus, sizeof(carStatusType), 0,
               (struct sockaddr *)&s_other, s_len);
    }
    close(s);
    pthread_exit(NULL);
}

void *command(void *param) {
    while (running) {
        updatePWM(&pwmA, carControl.speed, PWM_RAMP_RATE);
        updatePWM(&pwmB, carControl.direction, PWM_RAMP_RATE);
        usleep(50000);
    }
    pthread_exit(NULL);
}

void *PWMControl(void *param) {
    while (running) {
        gpioWrite(13, pwmA > 0);  // Example GPIO control
        gpioWrite(12, pwmA < 0);
        usleep(1000 * abs(pwmA));
    }
    pthread_exit(NULL);
}

