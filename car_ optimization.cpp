// 修改command函数，保持简单直接的控制逻辑
void *command(void *param) {
    while(running) {
        // 简单的速度和方向控制
        int leftSpeed, rightSpeed;
        
        // 直接将速度值应用到电机
        if(carControl.calibrate >= 0) {
            leftSpeed = carStatus.speed - carStatus.direction;
            rightSpeed = carStatus.speed + carStatus.direction;
        } else {
            leftSpeed = carStatus.speed - carStatus.direction;
            rightSpeed = carStatus.speed + carStatus.direction;
        }
        
        // 控制左电机
        if(leftSpeed > 0) {
            gpioWrite(servoA1, 1);
            gpioWrite(servoA2, 0);
            pwmA = abs(leftSpeed);
        } else if(leftSpeed < 0) {
            gpioWrite(servoA1, 0);
            gpioWrite(servoA2, 1);
            pwmA = abs(leftSpeed);
        } else {
            gpioWrite(servoA1, 0);
            gpioWrite(servoA2, 0);
            pwmA = 0;
        }
        
        // 控制右电机
        if(rightSpeed > 0) {
            gpioWrite(servoB1, 1);
            gpioWrite(servoB2, 0);
            pwmB = abs(rightSpeed);
        } else if(rightSpeed < 0) {
            gpioWrite(servoB1, 0);
            gpioWrite(servoB2, 1);
            pwmB = abs(rightSpeed);
        } else {
            gpioWrite(servoB1, 0);
            gpioWrite(servoB2, 0);
            pwmB = 0;
        }
        
        // 添加调试输出
        printf("\rLeft: %d (A1=%d,A2=%d) Right: %d (B1=%d,B2=%d)    ", 
            leftSpeed, gpioRead(servoA1), gpioRead(servoA2),
            rightSpeed, gpioRead(servoB1), gpioRead(servoB2));
        fflush(stdout);
        
        usleep(50000);  // 20Hz更新频率
    }
    
    // 停止电机
    gpioWrite(servoA1, 0);
    gpioWrite(servoA2, 0);
    gpioWrite(servoB1, 0);
    gpioWrite(servoB2, 0);
    
    return NULL;
}
