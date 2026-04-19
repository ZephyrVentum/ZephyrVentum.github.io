/**********************************************/                
/* Пример для работы #5                       */
/**********************************************/
/* СЕМАФОРЫ ПРОЦЕССОВ                         */
/**********************************************/
/* Монитор Слонов  - файл ganesha5.c          */
/**********************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>

#include "../common/elephant.h"
#include "../common/curtime.h"
#include "../common/ganesha.h"

/*static*/ int ne; // параметр циклов
/*static*/ meleph mel[NE]; // управляющая информация о Слонах
int cnt; // число запущенных Слонов
char chld_name[]="./elephant5";

/* управляющая структура для инициализации семафора */
union semun
{
	int val;
	//semid_ds *buf;
	unsigned short *array;
} arg;

/**********************************************/
main(int an, char *av[])
{
	int i,s; 
	int sk;      /* идентификатор семафора */
	/* значение для инициализации семафора */
	unsigned short val[1] = { 0 };
	/* структура для задания семафорной операции */
	struct sembuf sb[1];
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

	/* создание семафора */
	sk=semget(IPC_PRIVATE,1,IPC_CREAT|0x1ff);
	/* перевод идентификатора семафора в строковый вид,
	в таком виде он будет передан дочерним процессам */ 
	sprintf(t3,"%d",sk);
	*t4=0;
	/* инициализация семафора значением 0 */
	arg.array=val;
	semctl(sk,0,SETALL,arg);
	/* порождение дочерних процессов */
	for (ne=0; ne<NE; ne++)
	{
		/* запись личных данных в управляющую информацию */
		mel[ne].el=&ee[ne];
		/* представление нестроковых данных в строковом виде */
		sprintf(t1,"%d",mel[ne].el->age);
		sprintf(t2,"%lf",mel[ne].el->weight);
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
	/* пауза, чтобы процессы успели загрузить Слонов */
	sleep(1);
	/* перебор запущенных процессов */
	for (cnt=ne=0; ne<NE; ne++)
	{
		/* проверка - не закончился ли процесс */
		pw=waitpid(mel[ne].chpid,&stat,WNOHANG);
		if (pw==mel[ne].chpid) 
			/* если процесс закончился, значит, запуск Слона не удался */  
			printf("Слон %s не запущен\n",mel[ne].el->name);
		else
		{
			/* состояние процесса - запущен */
			mel[ne].status=0;
			/* подсчет запущенных Слонов */
			cnt++;
		}
	}
	/* если счетчик запущенных 0 - завершение Ганеши */
	if (!cnt) exit(0);

	/* выполнение в цикле V-операций, моделирующих выдачу порций воды */
	for (i=0; i<80; i++)
	{
		sb[0].sem_num=0;
		sb[0].sem_op=1;
		sb[0].sem_flg=SEM_UNDO;
		semop(sk,sb,1);
		usleep((long)(320.*rand()/RAND_MAX)*2000);
	}
	
	/* перебор дочерних процессов и прекращение тех, которые еще не закончились */
		
	for (ne=0; ne < NE; ne++)
	{
		pw = waitpid(mel[ne].chpid,&stat,WNOHANG);
		if (mel[ne].chpid!=pw)
		{
			/* если Слон не завершился, ему посылается сигнал KILL */  
			kill(mel[ne].chpid,SIGKILL);
			/* ожидание завершения после сигнала */
			waitpid(mel[ne].chpid,&stat,0);
			/* сообщение о гибели */
			printf("%s - Слон %s погиб\n",
			curtime(),mel[ne].el->name,stat);
		}
	}

	/* уничтожение семафора */
	semctl(sk,0,IPC_RMID,arg);
}
