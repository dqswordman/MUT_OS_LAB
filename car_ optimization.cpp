void *command(void *param) {
    while(running) {
        // 获取目标速度和方向
        int targetSpeed = -carStatus.speed;  // 反转速度方向，范围-100到100
        int targetDirection = carStatus.direction;  // 范围-40到40
        
        // 计算左右轮的速度
        int leftSpeed, rightSpeed;
        
        // 方向控制的比例系数，将±40的方向值映射到适当的转向强度
        float directionFactor = targetDirection / 40.0f;
        
        if(carControl.calibrate >= 0) {
            // 考虑到方向值范围是±40，需要调整转向强度
            leftSpeed = targetSpeed - (int)(targetSpeed * directionFactor * 0.5f);
            rightSpeed = targetSpeed + (int)(targetSpeed * directionFactor * 0.5f);
        } else {
            leftSpeed = targetSpeed - (int)(targetSpeed * directionFactor * 0.5f);
            rightSpeed = targetSpeed + (int)(targetSpeed * directionFactor * 0.5f);
        }
        
        // 速度平滑处理
        const float smoothFactor = 0.8f;  // 平滑系数
        static int lastLeftSpeed = 0;
        static int lastRightSpeed = 0;
        
        // 应用平滑
        leftSpeed = (int)(lastLeftSpeed * (1-smoothFactor) + leftSpeed * smoothFactor);
        rightSpeed = (int)(lastRightSpeed * (1-smoothFactor) + rightSpeed * smoothFactor);
        
        // 保存当前速度用于下次平滑
        lastLeftSpeed = leftSpeed;
        lastRightSpeed = rightSpeed;
        
        // 控制左电机
        if(leftSpeed > 0) {
            gpioWrite(servoA1, 0);  // 正转
            gpioWrite(servoA2, 1);
            pwmA = abs(leftSpeed);
        } else if(leftSpeed < 0) {
            gpioWrite(servoA1, 1);  // 反转
            gpioWrite(servoA2, 0);
            pwmA = abs(leftSpeed);
        } else {
            gpioWrite(servoA1, 0);  // 停止
            gpioWrite(servoA2, 0);
            pwmA = 0;
        }
        
        // 控制右电机
        if(rightSpeed > 0) {
            gpioWrite(servoB1, 0);  // 正转
            gpioWrite(servoB2, 1);
            pwmB = abs(rightSpeed);
        } else if(rightSpeed < 0) {
            gpioWrite(servoB1, 1);  // 反转
            gpioWrite(servoB2, 0);
            pwmB = abs(rightSpeed);
        } else {
            gpioWrite(servoB1, 0);  // 停止
            gpioWrite(servoB2, 0);
            pwmB = 0;
        }

        // 调试输出
        printf("\rSpd=%3d Dir=%3d | L=%3d(%d,%d) R=%3d(%d,%d) Mode=%d    ", 
            targetSpeed, targetDirection,
            leftSpeed, gpioRead(servoA1), gpioRead(servoA2),
            rightSpeed, gpioRead(servoB1), gpioRead(servoB2),
            carControl.autoMode);
        fflush(stdout);
        
        usleep(20000);  // 50Hz控制频率，匹配控制器更新频率
    }
    
    // 停止电机
    gpioWrite(servoA1, 0);
    gpioWrite(servoA2, 0);
    gpioWrite(servoB1, 0);
    gpioWrite(servoB2, 0);
    
    return NULL;
}
