void *command(void *param){
    while(running){
        if(carControl.calibrate>=0){
            // 所有速度值增加20%
            pwmA=((int)carStatus.speed*200)/100-(int)(carStatus.direction);
            pwmB=((int)carStatus.speed*160)/100+(int)(carStatus.direction);
        }else{
            pwmA=(int)(carStatus.speed*200/100)-(int)(carStatus.direction);
            pwmB=(int)(carStatus.speed*160/100)+(int)(carStatus.direction);
        }

        // PWM限制调高
        if(pwmA > 200) pwmA = 200;  // 允许更高的最大值
        if(pwmB > 160) pwmB = 160;

        usleep(25000);  // 减小延迟提高响应速度
    }
    pthread_exit(NULL);
}
