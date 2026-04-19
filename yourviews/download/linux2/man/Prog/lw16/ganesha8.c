/**********************************************/                
/* Пример для работы #8                       */
/**********************************************/
/* НИТИ И СЕМАФОРЫ НИТЕЙ                      */
/**********************************************/
/* Монитор Слонов И Слоны - файл ganesha8.c   */
/**********************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include "../common/elephant.h"
#include "../common/curtime.h"
#include "../common/ganesha.h"

#define PCOUNT 50

/*static*/ int ne; /* параметр циклов */
/*static*/ meleph mel[NE]; /* управляющая информация о Слонах */  
int cnt; /* число запущенных Слонов */
/*static*/ unsigned char Alarm=0;  /* признак поступления сигнала тревоги */

#define poolSize 48     /* размер пула */

/*static*/ char pool[poolSize];   /* пул */
/*static*/ int fullPos=poolSize;  /* индекс последней заполненной порции */  
/*static*/ pthread_mutex_t mutex; /* исключающий семафор */
/*static*/ sem_t count;           /* семафор - счетчик */
/*static*/ int pcount=0;

/* описания функций */
void *elephant8(meleph *this);
int el_full(int pos);
int el_alloc(int pos,char *n);
void el_free(int pos,char *n);

/**********************************************/
/* обработчик сигнала тревоги */
void hAlrm(int sn)
{
	Alarm=1;   /* установка признака */
}

/**********************************************/
/* функция main - модель деятельности Ганеши  */
int main()
{
	char *shm[NE];    /* адреса буферов */
	int need[NE],     /* потребности Слонов */
	last[NE];       /* позиции в буферах */
	pthread_t elId[NE]; /* идентификаторы нитей */
	int i;
   
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
	for (i=0; i<poolSize; pool[i++]=1);
	/* инициализация исключающего семафора */
	pthread_mutex_init(&mutex,NULL);
	/* инициализация семафора - счетчика */
	sem_init(&count,0,poolSize);

	/* порождение дочерних нитей */
	for (ne=0; ne<NE; ne++)
	{
		/* запись личных данных в управляющую информацию */
		mel[ne].el=&ee[ne];
		/* представление нестроковых данных в строковом виде */
		sprintf(t1,"%d",mel[ne].el->age);
		sprintf(t2,"%lf",mel[ne].el->weight);
		/* запуск функции elephant8 в нити */
		pthread_create(&elId[ne],NULL,(void *)&elephant8,(void *)(mel+ne));  
	}

	/* запуск будильника */
	alarm(5);
	/* цикл до сигнала будильника */
	while(!Alarm)
	{
		usleep(1000.*rand()/RAND_MAX);
		fullPos=el_full(fullPos);
		if (++pcount<=PCOUNT)
			printf("%s Заполнен буфер %d\n",curtime(), fullPos);
	}  
	printf("ALARM!!!-------------\n");
	/* прекращение нитей незавершившихся Слонов */
	for (ne=0; ne<NE; ne++) 
		if (mel[ne].status<2)
		{
			pthread_cancel(elId[ne]);
			printf("%s Слон %s погиб\n",curtime(),mel[ne].el->name);
		}
exit (0);
}

/**********************************************/
/* модель деятельности Слона                  */ 
void *elephant8(meleph *this)
{
	int need;       /* потребность */
	int lastind=-1; /* номер последней использованной порции */
	need=this->el->weight*this->el->weight*150;
	while (need)
	{
		/* получение порции */
		lastind=el_alloc(lastind,this->el->name);
		if (pcount<=PCOUNT)
			printf("%s Слон %s потребность - %d, буфер - %d\n",
		curtime(),this->el->name,need,lastind);
		/* потребление порции */
		usleep(10000/this->el->age+1);
		/* освобождение буфера */
		el_free(lastind,this->el->name);
		need--;
	}
	printf("%s Слон %s завершился\n",curtime(),this->el->name);
	this->status=2;
}

/**********************************************/
/* заполнение очередной порции в пуле */
int el_full(int pos)
{
	int p1;

	/* взаимное исключение на входе в монитор */
	pthread_mutex_lock(&mutex);
	/* номер следующего элемента */
	/* здесь и далее операция % - "остаток от деления" обеспечивает 
	перевод индекса в 0, когда он достигает значения poolSize */
	p1=pos=(pos++)%poolSize; 
	/* цикл поиска подходящего для заполнения элемента */
	while (1)
	{
		/* если элемент содержит 0 (пуст), то он заполняется */
		if (pool[pos]==0)
		{
			/* запись признака "заполнен" */
			pool[pos]=1; 
			/* увеличение на 1 семафора - счетчика заполненных */
			sem_post(&count);  
			break;
		}
		/* если элемент не подходит - переход к следующему */
		pos=(pos++)%poolSize;
		/* если сделан полный круг по массиву - безрезультатное завершение */
		if (pos==p1)
		{
			pos--;
			break;
		}
	}                   
	/* разблокировка на выходе из монитора */
	pthread_mutex_unlock(&mutex);
	/* возврат последнего заполненного элемента */
	return pos;
}

/**********************************************/
/* выделение очередной порции */
int el_alloc(int pos,char *n)
{
	/* уменьшение на 1 счетчика заполненных и ожидание, если в счетчике сейчас 0 */
	sem_wait(&count);  
	/* взаимное исключение на входе в монитор */
	pthread_mutex_lock(&mutex);
	/* номер следующего элемента */
	pos=(pos++)%poolSize; 
	/* цикл поиска подходящего элемента */
	/* подходящий элемент обязательно будет найден, поскольку счетчик заполненных был не 0 */
	while (1)
	{
		if (pool[pos]==1)
		{
			/* запись признака "выделен" */
			pool[pos]=-1;
			break;
		}
		pos=(pos++)%poolSize; 
	}
	/* выход из монитора */
	pthread_mutex_unlock(&mutex);
	/* возвращает номер выделенного элемента */
	return pos;
}
  
/**********************************************/
/* опустошение элемента */
void el_free(int pos,char *n)
{
	/* взаимное исключение на входе в монитор */
	pthread_mutex_lock(&mutex);
	/* запись признака "пуст" */
	pool[pos]=0;
	/* выход из монитора */
	pthread_mutex_unlock(&mutex);
}
