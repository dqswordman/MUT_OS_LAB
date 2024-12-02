void *command(void *param){
    while(running){
        if(carControl.calibrate>=0){
            // 只增加左轮功率来纠正偏右
            pwmA=((int)carStatus.speed*110)/100-(int)(carStatus.direction); // 增加左轮功率10%
            pwmB=(int)carStatus.speed+(int)(carStatus.direction);  // 右轮保持原功率
        }else{
            pwmA=(int)(carStatus.speed*110/100)-(int)(carStatus.direction);
            pwmB=(int)carStatus.speed+(int)(carStatus.direction);
        }
        
        // 限制PWM最大值确保安全
        if(pwmA > 100) pwmA = 100;
        if(pwmB > 100) pwmB = 100;
        
        usleep(20000);
    }
    pthread_exit(NULL);
}
