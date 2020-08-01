/**********************************************/
/* Пример для работы #1                       */
/**********************************************/
/* ПОРОЖДЕНИЕ ПРОЦЕССОВ                       */
/**********************************************/
/* Монитор Слонов  - файл ganesha1.c          */
/**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>

#include "../common/ganesha.h"
#include "../common/elephant.h"
#include "../common/curtime.h"

/*static*/ int ne;  /* параметр циклов */
/*static*/ meleph mel[NE]; /* управляющая информация о Слонах */
/*static*/ int cnt; /* число запущенных Слонов */

/*static*/ char chld_name[]="./elephant1";
/********************************************************/
main()
{
	int stat;  /* состояние процесса при завершении */
	/* строки для символьного представления параметров */  
	char t1[PARAMSTR_LENGTH], t2[PARAMSTR_LENGTH],
	     t3[PARAMSTR_LENGTH], t4[PARAMSTR_LENGTH];
	pid_t pw;   /* идентификатор процесса */
	char eee[ERRMES_LENGTH];   /* текст сообщения об ошибке */

	/* начало работы */          
	/* инициализация генератора случайных чисел */
	srand(time(NULL));
	printf("%s - Начало работы\n",curtime());
	*t3=*t4=0;
	/* цикл по массиву Слонов */
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
		mel[ne].prty=(int)(10.* rand()/RAND_MAX);  
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

	/* пауза монитора, Слоны в это время выполняются */
	sleep(5);
	printf("%s - ВРЕМЯ ОЖИДАНИЯ ИСТЕКЛО\n",curtime());

	/* перебор всех запущенных Слонов */
	for (ne=0; ne<NE; ne++)
	{
		/* если состояние процесса "не запущен", он не проверяется */
		if (mel[ne].status<0) continue;
		/* проверка завершения Слона с заданным ID, без ожидания завершения */
		pw=waitpid(mel[ne].chpid,&stat,WNOHANG);
		if (mel[ne].chpid==pw)
		{
			/* если Слон завершился - сообщение о его успешном завершении */
			printf("%s - Слон %s нормально завершился\n",
			curtime(),mel[ne].el->name);
		}
		else
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
}
