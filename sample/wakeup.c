#include <stdio.h>
#include <pthread.h>

_Bool lk,sleep_q,r_lock,r_want;
_Bool isRunning=1;

void* client(void* n){
sleep:
	while (lk!=0){}
	lk=1;
	while (r_lock==1){
		r_want=1;
		isRunning=0;
		lk=0;
		while(!isRunning){}
	}
progress:
	r_lock=1;
	lk=0;
	goto sleep;
}

void* server(void* n){
wakeup:
	r_lock=0;
	while (lk!=0){};
	if (r_want){
		while(sleep_q!=0){}
		sleep_q=1;
		r_want=0;
		while(lk!=0){};
		if (!isRunning) isRunning=1;
		sleep_q=0;
	}
	goto wakeup;
}


int main(){
	pthread_t p0,p1;
	short t;
	t=0;
	pthread_create(&p0,NULL,client,&t);
	pthread_create(&p1,NULL,server,&t);
	return 0;
}
