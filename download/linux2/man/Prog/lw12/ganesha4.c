/**********************************************/
/* Пример для работы #4                       */
/**********************************************/
/* ИМЕНОВАННЫЕ КАНАЛЫ                         */
/**********************************************/
/* Монитор Слонов  - файл ganesha4.c          */
/**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>

#include "../common/curtime.h"
#include "../common/elephant.h"
#include "../common/ganesha.h"

/*static*/ char chld_name[]="./elephant4";
/* Дескрипторы каналов Слонов, по которым Слонам ередается вода */
/*static*/ int fdE[NE];
/* Дескриптор и имя канала Ганеши, по которому Слоны рапортуют */
/*static*/ int fdG;
char gFname[]="Ganesha";
/* Индивидуальные потребности Слонов */
/*static*/ long need[NE];

#define NAME mel[ne].el->name

/*static*/ int ne; // параметр циклов
/*static*/ meleph mel[NE]; // управляющая информация о Слонах
int cnt; // число запущенных Слонов
                    
/**********************************************/
main()
{
	long port;       /* размер 1 порции воды */
	char ename[16];  /* текст доклада от Слона */
	int cc, i;
	int stat;  /* состояние процесса при завершении */
	/* строки для символьного представления параметров */  
	char t1[PARAMSTR_LENGTH], t2[PARAMSTR_LENGTH],
	     t3[PARAMSTR_LENGTH], t4[PARAMSTR_LENGTH];
	pid_t pw;   /* идентификатор процесса */
	char eee[ERRMES_LENGTH];   /* текст сообщения об ошибке */
 
	/* начало работы */
	srand(time(NULL));  
	printf("%s Начало работы\n",curtime());
             
	/* цикл по массиву Слонов */
	for (ne=0; ne<NE; ne++)
	{    
		/* запись личных данных в управляющую информацию */
		mel[ne].el=&ee[ne];
		/* представление нестроковых данных в строковом виде */
		sprintf(t1,"%d",mel[ne].el->age);
		sprintf(t2,"%lf",mel[ne].el->weight);
		/* создание канала для Слона */
		unlink(NAME);
		mknod(NAME,S_IFIFO|0x1b6,0);
		/* вычисление потребности для Слона */
		need[ne]=mel[ne].el->weight*500;
	}

	/* создание канала для Ганеши */
	unlink(gFname);
	mknod(gFname,S_IFIFO|0x1b6,0);*t3=*t4=0;
	/* запуск процессов-Слонов */
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

	/* открытие каналов для Слонов */
	
	//В этом месте ошибка в методичке! Необходимо исправить HTML-код.	
	for (ne=0; ne < NE; ne++)
	{
		fdE[ne] = open(NAME,O_WRONLY);
		printf("%s Канал для %s открыт - %d\n",curtime(),NAME,fdE[ne]);
	}

	/* пауза, чтобы Слоны успели открыть свои каналы */
	sleep(1);

	ne=0;
	/* цикл, пока не будет удовлетворена потребность стада */
	while (cnt>0)
	{
		if (need[ne])
		{
			/* получение очередной порции */
			port=2000.*rand()/RAND_MAX;  
			if (port>need[ne]) port=need[ne];
			/* передача порции в канал */
			write(fdE[ne],&port,sizeof(long));
			/* уменьшение общей потребности */
			need[ne]-=port;      
			if (!need[ne]) cnt--;
			/* задержка на поиск следующей порции */
			usleep((int)(2000.*rand()/RAND_MAX));  
		}
		if (++ne==NE) ne=0;
	} 
	/* пауза монитора, Слоны в это время выполняются */
	sleep(10);
	printf("ВРЕМЯ ОЖИДАНИЯ ИСТЕКЛО\n");

	/* открытие канала, по которому Слоны докладывают */
	fdG=open(gFname,O_RDONLY|O_NDELAY);
	/* пауза, чтобы Слоны подключились к другому концу канала */
	sleep(1);
	for (; ; )
	{
		/* чтение 1-й буквы очередного доклада */
		cc=read(fdG,ename,1);
		/* если конец данных в канале - конец приема докладов */
		if (cc<=0) break;
		/* чтение остального текста доклада - до конца строки */
		for (i=1; ; i++)
		{
			cc=read(fdG,ename+i,1);
			if (ename[i]==0) break;
		}
		printf("%s Слон %s доложил о завершении\n",curtime(),ename);
		/* Доклад Слона представляет собой его имя.
		Это имя ищется в списке Слонов, и Слон помечается законченным */  
		for (ne=0; ne<NE; ne++) 
			if (!strcmp(ename,NAME)) break;
		mel[ne].status=2;    
	}
	/*                 файл incl_trm.c                     */
	/* перебор дочерних процессов и прекращение тех,
	которые еще не закончились */
	
	//В этом месте ошибка в методичке! Необходимо исправить HTML-код.	
	for (ne=0; ne<NE; ne++)
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

	/* закрытие и уничтожение каналов */
	for (ne=0; ne<NE; ne++)
	{
		close(fdE[ne]);
		unlink(NAME);
	}
	close(fdG);
	unlink(gFname);
}
