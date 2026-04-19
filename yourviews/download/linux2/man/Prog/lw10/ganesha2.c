/**********************************************/
/* Пример для работы #2                       */
/**********************************************/
/* СИГНАЛЫ                                    */
/**********************************************/
/* Монитор Слонов  - файл ganesha2.c          */
/**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>

#include "../common/curtime.h"
#include "../common/ganesha.h"
#include "../common/elephant.h"

/*static*/ char chld_name[]="./elephant2";

/*static*/ int ne; /* параметр циклов */
/*static*/ meleph mel[NE]; /* управляющая информация о Слонах */  
int cnt; /* число запущенных Слонов */

void usrHandler(int sig)
{
	if (sig==SIGUSR1) mel[ne].status=1;
	else mel[ne].status=2;
}
/********************************************************/
main()
{
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

	/* установка обработчика сигналов */
	signal(SIGUSR1,usrHandler);
	signal(SIGUSR2,usrHandler);

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
		mel[ne].prty=(int)(10.*rand()/RAND_MAX);
		setpriority(PRIO_PROCESS,pw,mel[ne].prty);
		/* запоминание ID процесса-Слона */
		mel[ne].chpid=pw;
		// end of "incl_cal.c"
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
	printf("%s ВРЕМЯ ОЖИДАНИЯ ИСТЕКЛО\n",curtime());

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
			printf("%s Слон %s нормально завершился\n",
			curtime(),mel[ne].el->name);
			cnt--;
		}
		else
		{
			/* если Слон не завершился, ему посылается сигнал USR1 */
			if (kill(mel[ne].chpid,SIGUSR1)==0)
			{
				/* ожидание обработки ответного сигнала */
				while (!mel[ne].status);
				kill(mel[ne].chpid,SIGTERM);
				printf("%s Слону %s послан SIGTERM\n",curtime(),mel[ne].el->name);  
			}
			else cnt--;
		}
	}
	while (cnt)
	{
		pw=wait(&stat);
		for (ne=0; ne<NE; ne++)
			if (pw==mel[ne].chpid) break;
		if (stat)
			printf("%s Слон %s погиб\n",curtime(),mel[ne].el->name);
		else
			printf("%s Слон %s нормально завершился \n",
		curtime(),mel[ne].el->name);
		cnt--;
	}
}
