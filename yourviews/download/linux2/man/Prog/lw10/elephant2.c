/**********************************************/
/* Пример для работы N2                       */
/**********************************************/
/* Дочерний процесс, моделирующий поведение Слона    */
/**********************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <signal.h>

#include "../common/wait.h"
extern /*static*/ int waitFactor; // счетчик циклов ожидания и функции ожидания

#include "../common/curtime.h"
#include "../common/elephant.h"

/*static*/ elephant this;
/*static*/ int thisState;

/* обработчик сигнала SIGUSR1 */
void usr1Handler(int sig_num)
{
	if (!thisState)
		kill(getppid(),SIGUSR1);
	else	kill(getppid(),SIGUSR2);
}

/* обработчик сигнала SIGTERM */
void termHandler(int sig_num)
{
	if ( !thisState || (waitFactor>1000) )
		exit(8);  
	else	printf("%s Слон %s получил отсрочку (%d)\n",
	curtime(),this.name,waitFactor);
}

main(int an, char *av[])
{
	int need;
	int opt;

	/* установка обработчика сигнала SIGUSR1 */
	signal(SIGUSR1,usr1Handler);
	signal(SIGTERM,termHandler);

	/* прием параметров */
	strcpy(this.name,av[0]);
	sscanf(av[1],"%d",&(this.age)); 
	sscanf(av[2],"%lf",&(this.weight));

	/* инициализация генератора случайных чисел */  
	srand(time(NULL));
	/* состояние - 0 */
	thisState=0;

	/* сообщение о начале */
	printf("%s - Слон %s поиск начал, приоритет=%d\n",
	curtime(),this.name,getpriority(PRIO_PROCESS,getpid()));  

	/* время поиска воды - случайная величина, отчасти зависящая от возраста  */
	a1wait(10*(60-this.age));
	/* поиск окончен, состояние - 1 */
	thisState=1;

	/* сообщение об окончании поиска */
	printf("%s - Слон %s нашел воду\n",curtime(),this.name);  

	/* время насыщения зависит от веса Слона */
	a0wait((int)(this.weight*30));

	/* сообщение об окончании водопоя */
	printf("%s - Слон %s напился\n",curtime(),this.name);

	exit(0);
}
