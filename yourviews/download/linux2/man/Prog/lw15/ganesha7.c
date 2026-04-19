/**********************************************/                
/* Пример для работы #7                       */
/**********************************************/
/* ОЧЕРЕДИ СООБЩЕНИЙ                          */
/**********************************************/
/* Монитор Слонов  - файл ganesha7.c          */
/**********************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <setjmp.h>

#include "../common/elephant.h"
#include "../common/curtime.h"
#include "../common/ganesha.h"

/* размер текста в сообщении */
#define MSGSZ 30
/* типы сообщений */
char *mm1[]={"","NEED","SUCC","REST","FINISH"};
char *mm2[]={"","GRANT","TERM"};

/*static*/ int ne; /* параметр циклов */
/*static*/ meleph mel[NE]; /* управляющая информация о Слонах */  
int cnt; /* число запущенных Слонов */
/*static*/ char chld_name[]="./elephant7";
/*static*/ char Alarm=0; /* признак поступления сигнала тревоги */  
/*static*/ jmp_buf jb;           /* буфер состояния для longjmp */

/* обработчик сигнала тревоги */
void hAlrm(int sn)
{
	Alarm=1;   /* установка признака */
	longjmp(jb,1); /* "длинный" переход на начало цикла */
}

/**********************************************/
main()
{
	/* список резервных порций */
	struct pool
	{
		int size;
		struct pool *next;
	} *pList, *nList;
	char name[12];  /* имя из сообщения */
	int value;      /* числовое значение из сообщения */
	char *msgEl[NE];  /* ID очередей Слонов */
	int need[NE],     /* потребности Слонов */
	in[NE],       /* позиции в буферах */
	msgId[NE];    /* идентификаторы выходных очередей */
	int msgGaneshaId; /* идентификатор входнjq очередb */
	struct msgbuf
	{   /* буфер сообщения */
		long mtype;
		char mtext[MSGSZ];
	} message;
	int i, nmm, nem;
	int stat;  /* состояние процесса при завершении */
	/* строки для символьного представления параметров */  
	char t1[PARAMSTR_LENGTH], t2[PARAMSTR_LENGTH],
	     t3[PARAMSTR_LENGTH], t4[PARAMSTR_LENGTH];
	/* идентификаторы процессов */
	pid_t pw;
	/* текст сообщения об ошибке */
	char eee[ERRMES_LENGTH];

	/* начало работы */
	srand(time(NULL));  
	printf("%s Начало работы\n",curtime());
	/* установка обработчика сигнала тревоги */
	signal(SIGALRM,hAlrm);

	/* создание очереди, по которой будут приходить сообщения от всех Слонов */
	msgGaneshaId=msgget(IPC_PRIVATE,IPC_CREAT|0x1ff);
	/* преобразование идентификатора очереди в строку */
	sprintf(t4,"%d",msgGaneshaId);
	/* порождение дочерних процессов */
	for (ne=0; ne<NE; ne++)
	{
		/* запись личных данных в управляющую информацию */
		mel[ne].el=&ee[ne];
		/* представление нестроковых данных в строковом виде */
		sprintf(t1,"%d",mel[ne].el->age);
		sprintf(t2,"%lf",mel[ne].el->weight);
		/* создание очереди, по которой будут 
		отправляться сообщения Слону */
		msgId[ne]=msgget(IPC_PRIVATE,IPC_CREAT|0x1ff);
		if (msgId[ne]<0)
		{
			perror(eee);
			printf("%s\n",eee); 
			exit(0);
		}
		/* преобразование идентификатора очереди в строку */
		sprintf(t3,"%d",msgId[ne]);
		need[ne]=-1;
		/* порождение процесса */
		pw=fork();
		if (pw==0)
		{
			/* для дочернего процесса - запуск программы-Слона */  
			/* личные данные передаются через параметры */
			if (execl(chld_name, mel[ne].el->name, t1, t2, t3, t4, NULL)<0)
			{
				/* если загрузка программы-Слона почему-либо не удалась,
				печатается сообщение об ошибке, и процесс завершается */  
				perror(eee);
				printf("%s\n",eee); exit(0);
			}
			/* если вызов execl выполнился успешно, то далее в
			дочернем процессе выполняется программа elephant1 */
		}
		/* эта часть - только для процесса - Ганеши */
		/* заполнение управляющей информации о процессе  */
		/* состояние процесса пока что - не запущен */
		mel[ne].status=-1;
		/* установка случайной добавки к приоритету */
		mel[ne].prty=(int)(10.*rand()/RAND_MAX);
		setpriority(PRIO_PROCESS,pw,mel[ne].prty);
		/* запоминание ID процесса-Слона */
		mel[ne].chpid=pw;
	}

	pList=NULL;
	alarm(10);  /* подать сигнал тревоги через 30 сек */
	/* цикл пока не удовлетворятся все Слоны или не будет подан сигнал тревоги */
	setjmp(jb);
	while (!Alarm)
	{
		/* ожидание и прием сообщения */
		msgrcv(msgGaneshaId,&message,MSGSZ,0,0);
		printf("%s Ganesa: получено сообщение type=%s >%s<\n",
		curtime(),mm1[message.mtype],message.mtext);
		/* синтаксический разбор сообщения */
		if ((message.mtype==1)||(message.mtype==3))
			sscanf(message.mtext,"%s %d",name,&value);
		else
			strcpy(name,message.mtext);
		/* определение ne - Слона, который прислал сообщение */
		for (ne=0; ne<NE; ne++) 
			if (!strcmp(name,mel[ne].el->name)) break;
		/* идентификация типа сообщения */
		switch (message.mtype)
		{
			case 1: /* сообщение NEED */
				/* установка потребности Слона */
				need[ne]=value;
				/* если резерв непустой, посылка Слону сообщения типа GRANT */
				if (pList!=NULL)
				{
					message.mtype=1; sprintf(message.mtext,"%d",pList->size);/*&nbsp;&nbsp;*/
					msgsnd(msgId[ne],&message,MSGSZ,0);
					nList=pList; pList=pList->next; free(nList);
				}          
			break;
			case 2: /* сообщение SUCCESS */
				/* Слон не нуждается в воде (пока) */
				need[ne]=-1;
			break;
			case 3: /* сообщение REST */
				/* поиск нуждающегося Слона */
				for (nmm=ne=0,nem=-1; ne<NE; ne++)
					if (need[ne]>nmm)
						nmm=need[ne], nem=ne;
				if (nem>=0)
				{
					/* если таковой найден, ему посылается сообщение типа GRANT */
					message.mtype=1; sprintf(message.mtext,"%d",value);
					msgsnd(msgId[nem],&message,MSGSZ,0);
				} 
				/* если нуждающийся Слон не найден, порция заносится в резерв */
				else
				{
					nList=(struct pool*)malloc(sizeof(struct pool));
					nList->size=value;
					nList->next=pList;
					pList=nList;
				}       
			break;
			case 4: /* сообщение FINISH */
				/* признак завершения Слона */
				need[ne]=0;           
			break;
		}
	}
	printf("%s ВРЕМЯ ОЖИДАНИЯ ИСТЕКЛО\n",curtime());
	/* Слонам посылается сообщение TERM */
	for (ne=0; ne<NE; ne++)
	{
		if (need[ne])
		{
			message.mtype=2; strcpy(message.mtext,"TERM");
			msgsnd(msgId[ne],&message,5,0);
		}
	}
	/* уничтожение очередей */
	for (ne=0; ne<NE; ne++)
	{
		wait(&value);
		msgctl(msgId[ne],IPC_RMID,NULL);
	}
	msgctl(msgGaneshaId,IPC_RMID,NULL);
	/* освобождение списка резерва */
	while (pList!=NULL)
	{
		nList=pList->next; pList=nList; free(nList);
	}
}
