#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int flag[2];
int turn;
int test;

void* P0(void* t){
	flag[0]=1;
	turn=1;
	while (flag[1]==1 && turn==1){}
	test++;
	printf("%d\n",test);
	test--;
	flag[0]=0;
}

void* P1(void* t){
	flag[1]=1;
	turn=0;
	while (flag[0]==1 && turn==0){}
	test++;
	printf("%d\n",test);
	test--;
	flag[1]=0;
}

int main(){
	pthread_t p0,p1;
	short t;
	t=0;
	pthread_create(&p0,NULL,P0,t);
	pthread_create(&p1,NULL,P1,t);
	return 0;
}	
