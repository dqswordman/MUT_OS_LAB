void *command(void *param) {
    while (running) {
        if (carControl.calibrate >= 0) {
            // 直行速度倍率
            int baseSpeedA = ((int)carStatus.speed * 200) / 100;
            int baseSpeedB = ((int)carStatus.speed * 160) / 100;
            
            // 放大转弯影响（方向值权重增加）
            pwmA = baseSpeedA - ((int)(carStatus.direction) * 1.5);  // 转弯时左侧电机减速更多
            pwmB = baseSpeedB + ((int)(carStatus.direction) * 1.5);  // 转弯时右侧电机加速更多
        } else {
            int baseSpeedA = (int)(carStatus.speed * 200 / 100);
            int baseSpeedB = (int)(carStatus.speed * 160 / 100);

            pwmA = baseSpeedA - ((int)(carStatus.direction) * 1.5);
            pwmB = baseSpeedB + ((int)(carStatus.direction) * 1.5);
        }

        // 增加转弯响应幅度
        if (pwmA > 200) pwmA = 200;  // 左电机最大值
        if (pwmB > 200) pwmB = 200;  // 右电机最大值
        if (pwmA < -200) pwmA = -200;  // 允许更大的负值（反转）
        if (pwmB < -200) pwmB = -200;

        usleep(30000);  // 略微增加延迟，确保控制足够平稳
    }
    pthread_exit(NULL);
}
