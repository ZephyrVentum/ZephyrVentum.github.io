/**********************************************/
/* Пример для работы N3                       */
/**********************************************/
/* НЕИМЕНОВАННЫЕ ПРОГРАММНЫЕ КАНАЛЫ           */
/**********************************************/
/* Монитор Слонов  - файл ganesha3.c          */
/**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>
#include <setjmp.h>

#define MESS_LEN 100

#include "../common/curtime.h"
#include "../common/wait.h"
#include "../common/elephant.h"

/*static*/ int ne; // параметр циклов
/*static*/ meleph mel[NE]; // управляющая информация о Слонах
int cnt; // число запущенных Слонов

int alrmFlag;           // флаг поступления сигнала
int fd1[2], fd2[2];     // дескрипторы каналов
char mess[MESS_LEN];
jmp_buf jb;             // буфер состояния для longjmp

/**********************************************/
/* Эта функция выполняется в дочернем процессе и моделирует работу Слона */
/* Параметр функции - структура личных данных Слона */
void f_elephant(elephant *e)
{
	long need,    /* потребность Слона в воде */
	myport;  /* размер полученной порции */
	/* закрытие лишних канальных дескрипторов */
	close(fd2[0]); close(fd1[1]);
	need=e->weight*500;  /* вычисление потребности */
	printf("%s Слон %s приступил. Потребность - %ld, приоритет - %d\n",  
	curtime(),e->name,need,getpriority(PRIO_PROCESS,getpid()));
	/* цикл, пока не будет удовлетворена потребность */
	while (need>0)
	{
		/* ожидание и получение порции */
		read(fd1[0],&myport,sizeof(long));
		if (myport>need) myport=need;
		/* водопой */
		a0wait((int)(myport/e->age*10));
		/* уменьшение потребности */
		need-=myport;
		printf("%s Слон %s выпил %ld, остаток потребности - %ld\n",
		curtime(),e->name,myport,need);
	}
	printf("%s Слон %s водопой закончил\n",curtime(),e->name);
	/* вывод в канал доклада о завершении и завершение */
	write(fd2[1],e->name,strlen(e->name)+1);
	close(fd2[1]); close(fd1[0]);
	exit(0);
}

/**********************************************/
/* обработчик сигнала SIGALRM в родительском процессе */
void alrmH(int s)
{
	printf("%s ALARM!\n",curtime());
	alrmFlag=1;    /* установка флага */
	longjmp(jb,1); /* "длинный" переход на начало цикла опроса канала в функции main */
}
/**********************************************/

/* Функция main моделирует работу Ганеши      */
main()
{
	pid_t pw;    /* PID запущенного процесса */
	int s, i;
	long port;   /* размер очередной порции */
	long total;  /* общая потребность стада */

	pipe(fd1); pipe(fd2); /* создание каналов */
	/* цикл запуска Слонов */
	for (ne=0; ne<NE; ne++)
	{
		/* запись личных данных в управляющую информацию */
		mel[ne].el=&ee[ne];  
		/* порождение процесса */
		pw=fork();
		if (pw<0)
		{
			printf("pw=%d ne=%d\n",pw,ne);
			exit(0);
		}
		if (!pw) 
			/* в дочернем процессе запускается функция elephant */
			f_elephant(mel[ne].el);
		else
		{
			/* запоминание характеристик процесса-Слона */       
			mel[ne].status=-1;
			mel[ne].prty=(int)(10.*rand()/RAND_MAX);  
			setpriority(PRIO_PROCESS,pw,mel[ne].prty);
			mel[ne].chpid=pw;
		}
	}
	/* закрытие лишних канальных дескрипторов */
	close(fd2[1]); close(fd1[0]);
	/* инициализация генератора случайных чисел */
	srand(time(NULL));    
	/* вычисление общей потребности стада с запасом 20% */
	for(total=ne=0; ne<NE; ne++)
		total+=mel[ne].el->weight*1000;
	total=1.2*total;
	/* пауза - чтобы Слоны успели запуститься */
	sleep(1);
	/* цикл, пока не буден удовлетворена потребность стада */
	while(total>0)
	{
		/* получение очередной порции */
		port=1000.*rand()/RAND_MAX;  
		/* передача порции в канал */
		write(fd1[1],&port,sizeof(long));
		/* уменьшение общей потребности */
		total-=port;
		/* задержка на поиск следующей порции */
		usleep((int)(2000.*rand()/RAND_MAX));  
	}
	sleep(10);
	printf("%s ВРЕМЯ ОЖИДАНИЯ ИСТЕКЛО\n",curtime());
	/* отводится 1 сек на прием докладов от Слонов */
	signal(SIGALRM,alrmH);
	alarm(1);
	alrmFlag=0;
	/* запоминание точки возврата из обработчика сигнала SIGALRM */
	setjmp(jb);
	/* цикл, пока не поступит SIGALRM */
	while (!alrmFlag)
	{
		/* чтение из канала строки с именем Слона */
		for (i=0;;i++)
		{
			while(!read(fd2[0],mess+i,1));
			if (mess[i]==0) break;
		}
		/* поиск Слона по имени */
		for (ne=0; ne<NE; ne++) 
			if (!strcmp(mess,mel[ne].el->name)) break;      
		printf("%s Завершение слона %s зафиксировано\n",curtime(),mess);
		/* отметка о завершении */
		mel[ne].status=1;
	}
	/* закрытие каналов */
	close(fd2[0]); close(fd1[1]);
	/* перебор списка Слонов */
	for (ne=0; ne<NE; ne++)
	{
		/* тем, кто не доложил о завершении посылается SIGKILL */
		if (mel[ne].status!=1) kill(mel[ne].chpid,SIGKILL);
		/* Ганеша убеждается в завершении Слона */
		waitpid(mel[ne].chpid,&s,0);
		/* Сообщение о гибели, если Слон закончился с ненулевым кодом */
		if (s) printf("%s Зафиксирована гибель слона %s %d\n",
		curtime(),mel[ne].el->name,s);
	}
}
