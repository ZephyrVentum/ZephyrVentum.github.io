/**********************************************/
/* Пример для работы #7                       */
/**********************************************/
/* Дочерний процесс, моделирующий поведение Слона    */
/**********************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>

#include "../common/elephant.h"
#include "../common/curtime.h"
/* размер текста в сообщении */
#define MSGSZ 30
/* типы сообщений */
char *mm1[]={"","NEED","SUCC","REST","FINISH"};
char *mm2[]={"","GRANT","TERM"};

/*static*/ elephant this;  /* личные данные */
/*static*/ int thisState;         /* состояние */
/*static*/ struct msgbuf
	{  /* буфер сообщения */
		long mtype;
		char mtext[MSGSZ];
	} message;
/*static*/ int value;       /* величина порции */
/*static*/ int need;        /* потребность */
/*static*/ int msgGaneshaId;  /* идентификатор выходной очереди */
/*static*/ int msgMyId;       /* идентификатор входной очереди */ 

void success(void);  /* описание функции */
/**********************************************/
main(int an, char *av[])
{
	int i;
	/* прием параметров */
	strcpy(this.name,av[0]);
	sscanf(av[1],"%d",&(this.age)); 
	sscanf(av[2],"%lf",&(this.weight));

	/* инициализация генератора случайных чисел */  
	srand(time(NULL));
	/* состояние - 0 */
	thisState=0;

	sscanf(av[3],"%d",&msgMyId);
	sscanf(av[4],"%d",&msgGaneshaId);

	/* вычисление потребности */
	need=this.weight*this.weight*5;
	/* посылка сообщения NEED */
	message.mtype=1; 
	sprintf(message.mtext,"%s %d",this.name,need);
	msgsnd(msgGaneshaId,&message,MSGSZ,0);

	/* цикл, пока не будет получено сообщение TERM */
	while(need)
	{
		/* чтение сообщения TERM без ожидания */
		if (msgrcv(msgMyId,&message,MSGSZ,2,IPC_NOWAIT)>=0)
		{
			printf("%s %s: получено сообщение type=%s >%s<\n",
			curtime(),this.name,mm2[message.mtype],message.mtext);
			if (need) printf("%s Слон %s погиб\n",curtime(),this.name);  
			break;
		}
		/* чтение сообщения GRANT без ожидания */
		else
			if (msgrcv(msgMyId,&message,MSGSZ,1,IPC_NOWAIT)>=0)
			{
				printf("%s %s: получено сообщение type=%s >%s<\n",
				curtime(),this.name,mm2[message.mtype],message.mtext);
				sscanf(message.mtext,"%d",&value);
				success();
			}
			else
			{
				/* поиск воды */
				if (50.*rand()/RAND_MAX<2)
				{
					value=(50.*rand()/RAND_MAX)+1;
					printf("%s %s нашел -%d\n",curtime(),this.name,value);  
					success();
				}       
				else usleep(500);
			}
	}
	exit(0);
}
void success()
{
	int i;
	/* посылка сообщения SUCCESS */
	message.mtype=2; strcpy(message.mtext,this.name);
	msgsnd(msgGaneshaId,&message,MSGSZ,0);
	if (value>need)
	{
		/* посылка сообщения REST */
		message.mtype=3;
		sprintf(message.mtext,"%s %d",this.name,value-need);
		msgsnd(msgGaneshaId,&message,MSGSZ,0);
	}
	if (need)
	{
		if (value>need)
			i=need;
		else	i=value;
		usleep(2000000/this.age*i);
		need-=i;
		if (!need)
		{
			/* посылка сообщения FINISH */
			message.mtype=4; strcpy(message.mtext,this.name);
			msgsnd(msgGaneshaId,&message,MSGSZ,0);
			printf("%s Слон %s закончил водопой\n",
			curtime(),this.name);
		}        
		else
		{
			/* посылка сообщения NEED */
			message.mtype=1; 
			sprintf(message.mtext,"%s %d",this.name,need);
			msgsnd(msgGaneshaId,&message,MSGSZ,0);
		}        
	}
}
