/**********************************************/
/* Пример для работы N5                       */
/**********************************************/
/* Дочерний процесс, моделирующий поведение Слона    */
/**********************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <setjmp.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "../common/elephant.h"
#include "../common/wait.h"
#include "../common/curtime.h"

/*static*/ elephant this;  /* личные данные */
/*static*/ int thisState;         /* состояние */
/**********************************************/
main(int an, char *av[])
{
	int sk;        /* идентификатор семафора */
	/* структура для задания семафорной операции */
	struct sembuf sb[1];
	int need;        /* потребность */

	/* прием параметра - идентификатора семафора */
	/* прием параметров */
	strcpy(this.name,av[0]);
	sscanf(av[1],"%d",&(this.age)); 
	sscanf(av[2],"%lf",&(this.weight));
	/* инициализация генератора случайных чисел */  
	srand(time(NULL));
	/* состояние - 0 */
	thisState=0;
	sscanf(av[3],"%d",&sk);

	/* вычисление потребности */
	need=this.weight*this.weight;

	/* цикл, пока не будет удовлетворена потребность */  
	while (need)
	{ 
		printf("%s %s - %d\n",curtime(),this.name,need);
		/* P-операция над семафором */
		sb[0].sem_num=0;
		sb[0].sem_op=-1;
		sb[0].sem_flg=SEM_UNDO;
		semop(sk,sb,1);
		/* потребление и уменьшение потребности */
		a0wait(2000/this.age+1);
		need--;
	}
	/* нормальное завершение */
	printf("%s - Слон %s нормально завершился\n",
	curtime(),this.name);                  
	exit(0);  
}
