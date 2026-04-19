/**********************************************/
/* Пример для работы N6                       */
/**********************************************/
/* Дочерний процесс, моделирующий поведение Слона    */
/**********************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>

#include "../common/elephant.h"
#include "../common/curtime.h"

/*static*/ elephant this;  /* личные данные */
/*static*/ int thisState;         /* состояние */

#define shmSize ds.shm_segsz

/**********************************************/
main(int an, char *av[])
{
	int shmId;       /* идентификатор общей памяти */
	char *shm;       /* указатель на общую память */
	struct shmid_ds ds; /* управляющая структура памяти */
	int need;        /* потребность */
	int i;

	/* прием параметров */
	strcpy(this.name,av[0]);
	sscanf(av[1],"%d",&(this.age)); 
	sscanf(av[2],"%lf",&(this.weight));

	/* инициализация генератора случайных чисел */  
	srand(time(NULL));
	/* состояние - 0 */
	thisState=0;

	/* получение адреса общей памяти */
	sscanf(av[3],"%d",&shmId);
	shmctl(shmId,IPC_STAT,&ds);
	shm=(char *)shmat(shmId,(char *)0,0);

	/* вычисление потребности */
	need=this.weight*this.weight*5;

	/* цикл, пока не будет удовлетворена потребность */
	for (i=0; need; )
	{
		/* ожидание заполнения очередной порции */
		while(!shm[i]);
		/* выемка порции */
		shm[i]=0;
		/* использование общей памяти как кольцевого буфера */
		if (++i==shmSize) i=0;
		/* задержка на потребление */
		usleep(2000000/this.age+1);
		/* уменьшение потребности */
		need--;
		/* сообщение, когда потребность - круглое число */
		if (!(need%10)) 
			printf("%s - Слон %s : %d\n",curtime(),this.name,need);  
	}
	/* отсоединение общей памяти */
	shmdt(shm);
	/* нормальное завершение */
	printf("%s - Слон %s нормально завершился\n",
	curtime(),this.name);
	exit(0);
}
