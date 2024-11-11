For the two groups of code of MUT_OS_LAB, the full implementation

24/9/24 

  按照要求修正了Week6 Windows 和 Linux 四份文件的全部内容   
  
  修正了Week4_Linux_IntFile.cpp使用Week3共享结构体的问题 
  
  Modified all the contents of the four Week6 Windows and Linux documents as required.
  
  Fixed issue with Week4_Linux_IntFile.cpp using Week3 shared structure

1/10/24 

  经典的哲学家问题 如何把正常的代码干到死锁和三种不同的解决方案 半个小时后记：修复了方案一使用maxseats限制而不是真的少创建一个哲学家达到空一个座位的问题
  
  The classic philosopher's problem: How to make normal code deadlock and three different solutions. Half an hour postscript: Fixed the problem of using maxseats limit in solution 1 instead of creating one less philosopher to empty a seat.
  
  1.允许一个空座位 
    
    1.Allow one empty seat 
    
  2.使用非对称算法，强制奇数哲学家先选左边的筷子，再选右边的筷子，偶数哲学家先选右边的筷子，再选左边的筷子。

    2.Use asymmetric algorithm, by forcing the odd-numbered phylosopher to pick the chopstick on the left before on the right, and even-numbered phylosopher to pick the chopstick on the right 
    side before the left. 

  3.强制每个哲学家在给定的时间内选两根筷子。（一旦哲学家选了一根筷子，其他人就必须等到另一根筷子也被选上）

    3.Force each philosopher to pick both chopsticks at a given time. (Once a phylosopher picks a chopstick, everybody else has to wait until another chopstick is also picked)

11/11/24
  回到正轨，涵盖了期中考试后树莓派PI3前的绝大部分内容，以后内容及有空闲时间会具体注释代码对应的PI实验标题
