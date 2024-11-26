void *command(void *param) {
    while(running) {
        // 获取目标速度和方向
        int targetSpeed = -carStatus.speed; // 反转速度方向
        int targetDirection = carStatus.direction;
        
        // 计算左右轮的速度
        int leftSpeed, rightSpeed;
        
        // 基础速度分配
        if(carControl.calibrate >= 0) {
            leftSpeed = targetSpeed - targetDirection;
            rightSpeed = targetSpeed + targetDirection;
        } else {
            leftSpeed = targetSpeed - targetDirection;
            rightSpeed = targetSpeed + targetDirection;
        }
        
        // 平滑处理复合动作
        // 当同时有速度和转向时，通过系数调整来保持更流畅的动作
        if(targetSpeed != 0 && targetDirection != 0) {
            float smoothFactor = 0.7f; // 平滑系数
            leftSpeed = (int)(leftSpeed * smoothFactor);
            rightSpeed = (int)(rightSpeed * smoothFactor);
        }
        
        // 控制左电机
        if(leftSpeed > 0) {
            gpioWrite(servoA1, 0); // 反转控制信号
            gpioWrite(servoA2, 1);
            pwmA = abs(leftSpeed);
        } else if(leftSpeed < 0) {
            gpioWrite(servoA1, 1);
            gpioWrite(servoA2, 0);
            pwmA = abs(leftSpeed);
        } else {
            gpioWrite(servoA1, 0);
            gpioWrite(servoA2, 0);
            pwmA = 0;
        }
        
        // 控制右电机
        if(rightSpeed > 0) {
            gpioWrite(servoB1, 0); // 反转控制信号
            gpioWrite(servoB2, 1);
            pwmB = abs(rightSpeed);
        } else if(rightSpeed < 0) {
            gpioWrite(servoB1, 1);
            gpioWrite(servoB2, 0);
            pwmB = abs(rightSpeed);
        } else {
            gpioWrite(servoB1, 0);
            gpioWrite(servoB2, 0);
            pwmB = 0;
        }

        // 添加简单的速度渐变
        // 确保PWM值在合理范围内
        pwmA = (pwmA > 100) ? 100 : pwmA;
        pwmB = (pwmB > 100) ? 100 : pwmB;
        
        // 调试输出
        printf("\rSpeed=%d Dir=%d | Left=%d(%d,%d) Right=%d(%d,%d)    ", 
            targetSpeed, targetDirection,
            leftSpeed, gpioRead(servoA1), gpioRead(servoA2),
            rightSpeed, gpioRead(servoB1), gpioRead(servoB2));
        fflush(stdout);
        
        usleep(20000);  // 提高更新频率到50Hz
    }
    
    // 停止电机
    gpioWrite(servoA1, 0);
    gpioWrite(servoA2, 0);
    gpioWrite(servoB1, 0);
    gpioWrite(servoB2, 0);
    
    return NULL;
}
