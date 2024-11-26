#include <pigpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <math.h>

#pragma pack(1)

// 基础定义
#define FRONT   0
#define BACK    1
#define LEFT    2
#define RIGHT   3

#define PORT 8888

// GPIO定义
#define servoC          5       // Servo 90g LEFT
#define servoD          6       // Servo 90g RIGHT
#define servoA1         13      // Motor LEFT AIN1
#define servoA2         12      // Motor LEFT AIN2
#define servoB1         17      // Motor RIGHT BIN1
#define servoB2         16      // Motor RIGHT BIN2

// 控制参数
#define PWM_FREQUENCY   1000    // PWM频率
#define PWM_RANGE      1000     // PWM范围
#define CONTROL_INTERVAL 10000  // 控制间隔(微秒)
#define MOTOR_UPDATE_RATE 100   // 电机更新频率(Hz)

#define USDETECT        6       // 超声波检测距离
#define USMIN           3       // 最小安全距离

// 新增配置结构体
typedef struct {
    float speedLimit;          // 速度限制
    float accelerationRate;    // 加速率
    float decelerationRate;    // 减速率
    float turningSensitivity;  // 转向灵敏度
    float obstacleThreshold;   // 障碍物阈值
    float pidKp;              // PID比例系数
    float pidKi;              // PID积分系数
    float pidKd;              // PID微分系数
} CarConfig;

// 电机控制结构体
typedef struct {
    float currentSpeed;    // 当前速度
    float targetSpeed;     // 目标速度
    float acceleration;    // 加速度
    float deceleration;    // 减速度
    int pwmPin1;          // PWM引脚1
    int pwmPin2;          // PWM引脚2
} MotorControl;

// PID控制器结构体
typedef struct {
    float kp, ki, kd;
    float integral;
    float lastError;
    float outputLimit;
} PIDController;

// 原有状态结构体优化
typedef struct {
    uint8_t distance[4];
    int8_t  speed;
    int8_t  direction;
    uint8_t autoMode;
    uint8_t sensorSearch;
    uint16_t CRC;
    float actualSpeed;      // 实际速度
    float actualDirection;  // 实际方向
} carStatusType;

// 原有控制结构体
typedef struct {
    int8_t speed;
    int8_t direction;
    int8_t calibrate;
    uint8_t autoMode;
    uint8_t sensorSearch;
    uint8_t dummy;
    uint16_t CRC;
} carControlType;

// 全局变量
volatile int pwmA=0, pwmB=0;
volatile int pwmC=50, pwmD=50;
uint8_t running=true;
carStatusType carStatus = {{0}, 0, 0, 0, 0, 0, 0, 0};
carControlType carControl = {0, 0, 0, 0, 0};

// 传感器配置
int sensorTrig[4] = {20, 22, 24, 26};
int sensorEcho[4] = {21, 23, 25, 27};

// 全局配置
CarConfig config = {
    .speedLimit = 100.0f,
    .accelerationRate = 0.2f,
    .decelerationRate = 0.3f,
    .turningSensitivity = 1.0f,
    .obstacleThreshold = USDETECT,
    .pidKp = 1.0f,
    .pidKi = 0.1f,
    .pidKd = 0.05f
};

// 电机控制实例
MotorControl leftMotor = {0, 0, 0, 0, servoA1, servoA2};
MotorControl rightMotor = {0, 0, 0, 0, servoB1, servoB2};
PIDController directionPID = {0, 0, 0, 0, 0, 100.0f};

// 函数声明
void initGPIO(void);
void gpioStop(int sig);
void updateMotorSpeed(MotorControl *motor);
float updatePID(PIDController *pid, float error);
void setPWM(int pin, int value);
void *ultraSonic(void *param);
void *networkControl(void *param);
void *command(void *param);
void *autoControl(void *param);
void *PWMc(void *param);
void *PWMd(void *param);

// 电机控制函数实现
void updateMotorSpeed(MotorControl *motor) {
    float speedDiff = motor->targetSpeed - motor->currentSpeed;
    float acceleration = (speedDiff > 0) ? 
        config.accelerationRate : config.decelerationRate;
    
    if(fabs(speedDiff) > acceleration) {
        motor->currentSpeed += (speedDiff > 0) ? acceleration : -acceleration;
    } else {
        motor->currentSpeed = motor->targetSpeed;
    }
    
    // 限制速度范围
    if(motor->currentSpeed > config.speedLimit)
        motor->currentSpeed = config.speedLimit;
    else if(motor->currentSpeed < -config.speedLimit)
        motor->currentSpeed = -config.speedLimit;
}

// PID控制器更新
float updatePID(PIDController *pid, float error) {
    pid->integral += error;
    float derivative = error - pid->lastError;
    pid->lastError = error;
    
    float output = pid->kp * error + 
                  pid->ki * pid->integral + 
                  pid->kd * derivative;
                  
    // 限制输出范围
    if(output > pid->outputLimit)
        output = pid->outputLimit;
    else if(output < -pid->outputLimit)
        output = -pid->outputLimit;
        
    return output;
}

// PWM设置函数
void setPWM(int pin, int value) {
    value = (value < 0) ? 0 : ((value > PWM_RANGE) ? PWM_RANGE : value);
    gpioPWM(pin, value);
}

// 主函数
int main() {
    int i;
    pthread_t tid[8];
    pthread_attr_t attr[8];
    void *(*threads[8])(void *) = {
        ultraSonic, ultraSonic, ultraSonic, ultraSonic,
        networkControl, command, autoControl, PWMc
    };
    int threadsParam[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    // 初始化
    initGPIO();
    signal(SIGINT, gpioStop);

    // 初始化PID控制器
    directionPID.kp = config.pidKp;
    directionPID.ki = config.pidKi;
    directionPID.kd = config.pidKd;

    // 创建线程
    for(i = 0; i < 8; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], threads[i], &threadsParam[i]);
    }

    // 主循环 - 状态显示
    while(running) {
        printf("\rStatus: Speed=%d(%d) Dir=%d(%d) Mode=%d Dist[F:%d B:%d L:%d R:%d]",
            (int)carStatus.actualSpeed, carStatus.speed,
            (int)carStatus.actualDirection, carStatus.direction,
            carStatus.autoMode,
            carStatus.distance[FRONT], carStatus.distance[BACK],
            carStatus.distance[LEFT], carStatus.distance[RIGHT]);
        fflush(stdout);
        usleep(100000);
    }

    // 等待线程结束
    for(i = 0; i < 8; i++) {
        pthread_join(tid[i], NULL);
    }
    
    gpioTerminate();
    return 0;
}

// 命令处理线程
void *command(void *param) {
    while(running) {
        // 计算目标速度和方向
        float targetSpeed = (float)carStatus.speed;
        float targetDirection = (float)carStatus.direction;
        
        // 应用PID控制
        float directionError = targetDirection - carStatus.actualDirection;
        float directionOutput = updatePID(&directionPID, directionError);
        
        // 更新电机目标速度
        leftMotor.targetSpeed = targetSpeed - directionOutput;
        rightMotor.targetSpeed = targetSpeed + directionOutput;
        
        // 更新电机速度
        updateMotorSpeed(&leftMotor);
        updateMotorSpeed(&rightMotor);
        
        // 设置PWM输出
        if(leftMotor.currentSpeed >= 0) {
            setPWM(leftMotor.pwmPin1, (int)(leftMotor.currentSpeed));
            setPWM(leftMotor.pwmPin2, 0);
        } else {
            setPWM(leftMotor.pwmPin1, 0);
            setPWM(leftMotor.pwmPin2, (int)(-leftMotor.currentSpeed));
        }
        
        if(rightMotor.currentSpeed >= 0) {
            setPWM(rightMotor.pwmPin1, (int)(rightMotor.currentSpeed));
            setPWM(rightMotor.pwmPin2, 0);
        } else {
            setPWM(rightMotor.pwmPin1, 0);
            setPWM(rightMotor.pwmPin2, (int)(-rightMotor.currentSpeed));
        }
        
        // 更新实际状态
        carStatus.actualSpeed = (leftMotor.currentSpeed + rightMotor.currentSpeed) / 2;
        carStatus.actualDirection = (rightMotor.currentSpeed - leftMotor.currentSpeed) / 2;
        
        usleep(CONTROL_INTERVAL);
    }
    return NULL;
}

// 自动控制线程优化
void *autoControl(void *param) {
    while(running) {
        if(carControl.autoMode == 0) {
            // 手动模式
            carStatus.speed = carControl.speed;
            carStatus.direction = carControl.direction;
        } else {
            // 自动模式
            float frontDist = (float)carStatus.distance[FRONT];
            float backDist = (float)carStatus.distance[BACK];
            float leftDist = (float)carStatus.distance[LEFT];
            float rightDist = (float)carStatus.distance[RIGHT];
            
            // 计算安全系数
            float frontSafety = (frontDist - USMIN) / (float)(USDETECT - USMIN);
            frontSafety = fmax(0.0f, fmin(1.0f, frontSafety));
            
            float backSafety = (backDist - USMIN) / (float)(USDETECT - USMIN);
            backSafety = fmax(0.0f, fmin(1.0f, backSafety));
            
            // 根据行驶方向选择安全系数
            float speedSafety = (carControl.speed >= 0) ? frontSafety : backSafety;
            
            // 计算转向安全系数
            float turnBias = (rightDist - leftDist) / (float)USDETECT;
            float turnSafety = fmax(-1.0f, fmin(1.0f, turnBias));
            
            // 更新速度和方向
            carStatus.speed = (int)(carControl.speed * speedSafety);
            carStatus.direction = carControl.direction + 
                                (int)(turnSafety * config.turningSensitivity * 100);
        }
        
        usleep(10000);
    }
    return NULL;
}

// 网络控制线程保持不变
void *networkControl(void *param) {
    struct sockaddr_in s_me, s_other;
    int s, rLen, s_len = sizeof(s_other);
    
    if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("Cannot start UDP server\n");
        running = false;
        return NULL;
    }
    
    memset((char *)&s_me, 0, sizeof(s_me));
    s_me.sin_family = AF_INET;
    s_me.sin_port = htons(PORT);
    s_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(s, (struct sockaddr *)&s_me, sizeof(s_me)) == -1) {
        printf("Cannot bind UDP port\n");
        close(s);
        running = false;
        return NULL;
    }
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("Cannot set UDP timeout\n");
        close(s);
        running = false;
        return NULL;
    }
    
    while(running) {
        if((rLen = recvfrom(s, &carControl, sizeof(carControlType),
            0, (struct sockaddr *)&s_other, (socklen_t *)&s_len)) < 0) {
            continue;
        }
        
        if(sendto(s, &carStatus, sizeof(carStatusType),
            0, (struct sockaddr *)&s_other, s_len) == -1) {
            printf("Error sending data\n");
            running = false;
        }
    }
    
    close(s);
    return NULL;
}

// 超声波传感器线程
// 继续超声波传感器线程
void *ultraSonic(void *param) {
    int id = *(int *)param;
    struct timespec start, end;
    int timeout;
    float distance;
    
    while(running) {
        // 处理传感器搜索模式的舵机控制
        if(carControl.sensorSearch == 1) {
            if(id == LEFT || id == RIGHT) {
                static int scanCount = 0;
                if(scanCount >= 20) scanCount = 0;
                
                int servoAngle;
                if(scanCount < 10) {
                    servoAngle = (scanCount * 80) / 10 + 10;
                } else {
                    servoAngle = ((20 - scanCount) * 80) / 10 + 10;
                }
                
                if(id == LEFT) pwmC = servoAngle;
                if(id == RIGHT) pwmD = servoAngle;
                
                scanCount++;
            }
        } else {
            // 非搜索模式下的固定位置
            if(id == LEFT) pwmC = 10;
            if(id == RIGHT) pwmD = 90;
        }

        // 发送超声波脉冲
        gpioWrite(sensorTrig[id], 1);
        usleep(10);
        gpioWrite(sensorTrig[id], 0);
        
        // 等待回波开始
        clock_gettime(CLOCK_MONOTONIC, &start);
        timeout = 0;
        while(gpioRead(sensorEcho[id]) == 0 && timeout < 30000) {
            timeout++;
            usleep(1);
        }
        
        if(timeout >= 30000) {
            carStatus.distance[id] = 255; // 超时，设置最大距离
            continue;
        }
        
        // 等待回波结束
        clock_gettime(CLOCK_MONOTONIC, &end);
        timeout = 0;
        while(gpioRead(sensorEcho[id]) == 1 && timeout < 30000) {
            timeout++;
            usleep(1);
        }
        
        if(timeout < 30000) {
            // 计算时间差
            long diffInNanos = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
            // 声速340m/s, 来回距离除以2, 转换为厘米
            distance = diffInNanos / 58000.0f;  // (340 * 100 * nanos) / (2 * 1000000000)
            
            if(distance <= 400) { // 最大测量距离400cm
                carStatus.distance[id] = (uint8_t)distance;
            } else {
                carStatus.distance[id] = 255;
            }
        }
        
        usleep(50000); // 50ms测量间隔
    }
    return NULL;
}

// 左舵机PWM控制
void *PWMc(void *param) {
    int pulseWidth;
    int oldPWM = 50;
    int count = 0;
    
    while(running) {
        if(oldPWM == pwmC) {
            count++;
            if(count > 20) {
                count = 0;
            } else {
                usleep(20000);
                continue;
            }
        }
        oldPWM = pwmC;
        
        // 将0-100的值转换为舵机脉冲宽度(1000-2000μs)
        pulseWidth = pwmC * 10 + 1000;
        
        gpioServo(servoC, pulseWidth);
        usleep(20000); // 50Hz更新频率
    }
    
    gpioServo(servoC, 0); // 停止舵机
    return NULL;
}

// 右舵机PWM控制
void *PWMd(void *param) {
    int pulseWidth;
    int oldPWM = 50;
    int count = 0;
    
    while(running) {
        if(oldPWM == pwmD) {
            count++;
            if(count > 20) {
                count = 0;
            } else {
                usleep(20000);
                continue;
            }
        }
        oldPWM = pwmD;
        
        // 将0-100的值转换为舵机脉冲宽度(1000-2000μs)
        pulseWidth = pwmD * 10 + 1000;
        
        gpioServo(servoD, pulseWidth);
        usleep(20000); // 50Hz更新频率
    }
    
    gpioServo(servoD, 0); // 停止舵机
    return NULL;
}

// GPIO初始化函数
void initGPIO(void) {
    if(gpioInitialise() < 0) {
        printf("pigpio initialization failed\n");
        exit(1);
    }
    
    // 设置超声波传感器引脚
    for(int i = 0; i < 4; i++) {
        gpioSetMode(sensorTrig[i], PI_OUTPUT);
        gpioSetMode(sensorEcho[i], PI_INPUT);
        gpioSetPullUpDown(sensorEcho[i], PI_PUD_DOWN);
        gpioWrite(sensorTrig[i], 0);
    }
    
    // 设置电机控制引脚
    gpioSetMode(servoA1, PI_OUTPUT);
    gpioSetMode(servoA2, PI_OUTPUT);
    gpioSetMode(servoB1, PI_OUTPUT);
    gpioSetMode(servoB2, PI_OUTPUT);
    
    // 设置舵机引脚
    gpioSetMode(servoC, PI_OUTPUT);
    gpioSetMode(servoD, PI_OUTPUT);
    
    // 初始化PWM
    gpioSetPWMfrequency(servoA1, PWM_FREQUENCY);
    gpioSetPWMfrequency(servoA2, PWM_FREQUENCY);
    gpioSetPWMfrequency(servoB1, PWM_FREQUENCY);
    gpioSetPWMfrequency(servoB2, PWM_FREQUENCY);
    
    gpioSetPWMrange(servoA1, PWM_RANGE);
    gpioSetPWMrange(servoA2, PWM_RANGE);
    gpioSetPWMrange(servoB1, PWM_RANGE);
    gpioSetPWMrange(servoB2, PWM_RANGE);
    
    // 初始化所有输出为0
    gpioWrite(servoA1, 0);
    gpioWrite(servoA2, 0);
    gpioWrite(servoB1, 0);
    gpioWrite(servoB2, 0);
    gpioServo(servoC, 0);
    gpioServo(servoD, 0);
}

// 信号处理函数
void gpioStop(int sig) {
    printf("\nReceived signal %d, stopping...\n", sig);
    running = false;
    
    // 停止所有电机和舵机
    gpioWrite(servoA1, 0);
    gpioWrite(servoA2, 0);
    gpioWrite(servoB1, 0);
    gpioWrite(servoB2, 0);
    gpioServo(servoC, 0);
    gpioServo(servoD, 0);
}

// 辅助函数：限制值在指定范围内
float constrain(float value, float min, float max) {
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

// 辅助函数：计算CRC校验
uint16_t calculateCRC(void* data, size_t length) {
    uint16_t crc = 0xFFFF;
    uint8_t* bytes = (uint8_t*)data;
    
    for(size_t i = 0; i < length - 2; i++) {
        crc ^= (uint16_t)bytes[i] << 8;
        for(uint8_t j = 0; j < 8; j++) {
            if(crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}
