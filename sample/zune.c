#include <stdio.h>
#include <pthread.h>

#define IsLeapYear(year) (((year%4==0) && (year%100!=0)) || (year%400==0))

void* year(void* day){
	int days=*(int*)day;
	int year=1980;
	while (days>365){
		if (IsLeapYear(year)){
			if (days>366){
				days-=366;
				year+=1;
			}
		}else{
			days-=365;
			year+=1;
		}
	}
	printf("year:%d day:%d\n",year,days);
}

int main(){
	pthread_t y1,y2,y3;
	short i;
	for (i=365;i<368;i++){
		pthread_create(&y1,NULL,year,i);
	}
	return 0;
}

