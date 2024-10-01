/*
sem_wait(chopstick[no]);  // 哲学家拿左边的筷子
gotoxy(1,no*4+1);
printf("Mr. %c is taking a chopstick on the left side...\n",'A'+no);fflush(stdout);

sem_wait(chopstick[(no+1)%CHAIRNUM]);  // 哲学家拿右边的筷子
gotoxy(1,no*4+2);
printf("Mr. %c is taking a chopstick on the right side...\n",'A'+no);fflush(stdout);
*/


/*
sem_wait(chopstick[no]);  // 哲学家拿左边的筷子
gotoxy(1,no*4+1);
printf("Mr. %c is taking a chopstick on the left side...\n",'A'+no);fflush(stdout);

randomDelay();  // 引入延时，模拟占有并等待情况

sem_wait(chopstick[(no+1)%CHAIRNUM]);  // 哲学家拿右边的筷子
gotoxy(1,no*4+2);
printf("Mr. %c is taking a chopstick on the right side...\n",'A'+no);fflush(stdout);
*/
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "C:\Users\Administrator\Desktop\lab\lab\winconio.h"

#define CHAIRNUM 5

HANDLE chopstick[CHAIRNUM];
int timeUsed[CHAIRNUM];

void randomDelay(void);
DWORD WINAPI philosopher(LPVOID who);
DWORD sem_wait(HANDLE sem);
DWORD sem_signal(HANDLE sem);

int main(void) {
	int i;
	DWORD tid[CHAIRNUM];				// Thread ID
	HANDLE th[CHAIRNUM];				// Thread Handle
	int param[CHAIRNUM];

	clrscr();
	for (i = 0; i < CHAIRNUM; i++) {
		chopstick[i] = CreateSemaphore(
			NULL,   // default security attributes
			1,   // initial count
			1,   // maximum count
			NULL);  // unnamed semaphore
		param[i] = i;
		timeUsed[i] = 0;
	}

	// Create n threads
	for (i = 0; i < CHAIRNUM; i++)
		th[i] = CreateThread(
			NULL,				// Default security attributes
			0,					// Default stack size
			philosopher,			// Thread function
			(void*)&param[i],	// Thread function parameter
			0,					// Default creation flag
			&tid[i]);			// Thread ID returned.

	// Wait until all threads finish
	for (i = 0; i < CHAIRNUM; i++)
		if (th[i] != NULL)
			WaitForSingleObject(th[i], INFINITE);

	for (i = 0; i < CHAIRNUM; i++)
		CloseHandle(chopstick[i]);
	return 0;
}

void randomDelay(void) {
	int stime = ((rand() % 2000) + 100);
	Sleep(stime);
}


DWORD WINAPI philosopher(LPVOID who) {
	int no, i = 0;

	no = (int)*((int*)who);

	for (i = 0; i < 10; i++) {
		gotoxy(1, no * 4 + 1);
		printf("Mr. %c is thinking...                           \n", 'A' + no); fflush(stdout);
		randomDelay();

		sem_wait(chopstick[no]);
		randomDelay();  // 引入延时，模拟占有并等待情况
		gotoxy(1, no * 4 + 1);
		printf("Mr. %c is taking a chopstick on the left side...    \n", 'A' + no); fflush(stdout);
		sem_wait(chopstick[(no + 1) % CHAIRNUM]);
		gotoxy(1, no * 4 + 2);
		printf("Mr. %c is taking a chopstick on the right side...\n", 'A' + no); fflush(stdout);

		// Critical Section
//	if(!no)clrscr();  // To reduced garbage from writing in the wrong location
		randomDelay();
		timeUsed[no]++;
		timeUsed[(no + 1) % CHAIRNUM]++;
		gotoxy(no * 10 + 1, CHAIRNUM * 4);
		printf("CH[%d]=%d ", no, timeUsed[no]);
		gotoxy(((no + 1) % CHAIRNUM) * 10 + 1, CHAIRNUM * 4);
		printf("CH[%d]=%d ", (no + 1) % CHAIRNUM, timeUsed[(no + 1) % CHAIRNUM]);
		gotoxy(no * 10 + 1, CHAIRNUM * 4 + 1);
		printf("  [%c]=%d ", 'A' + no, i); fflush(stdout);

		gotoxy(1, no * 4 + 1);
		printf("                                                     \n");
		gotoxy(1, no * 4 + 2);
		printf("                                                     \n"); fflush(stdout);
		randomDelay();

		sem_signal(chopstick[no]);
		gotoxy(1, no * 4 + 1);
		printf("Mr. %c drops a chopstick on the left side...    \n", 'A' + no); fflush(stdout);
		sem_signal(chopstick[(no + 1) % CHAIRNUM]);
		gotoxy(1, no * 4 + 2);
		printf("Mr. %c drops a chopstick on the right side...     \n", 'A' + no); fflush(stdout);
		// Remaining Section
		randomDelay();
		gotoxy(1, no * 4 + 2);
		printf("                                                       \n");
		gotoxy(1, no * 4 + 1);
		printf("Mr. %c is chewing food...                             \n", 'A' + no); fflush(stdout);
		randomDelay();
	}
	gotoxy(1, no * 4 + 1);
	printf("Mr. %c is full...                                         \n", 'A' + no); fflush(stdout);
	return 0;
}

DWORD sem_wait(HANDLE sem) {
	DWORD result = WaitForSingleObject(sem, INFINITE);

	switch (result) {
	case WAIT_OBJECT_0:return 1;
	case WAIT_TIMEOUT: return 0;
	}
	return 0;
}

DWORD sem_signal(HANDLE sem) {
	return ReleaseSemaphore(sem, 1, NULL); // Increase by one
}
