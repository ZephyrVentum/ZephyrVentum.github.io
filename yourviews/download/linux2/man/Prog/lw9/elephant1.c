/**********************************************/
/* Пример для работы #1                       */
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

#include "../common/elephant.h"
#include "../common/wait.h"
#include "../common/curtime.h"

/*static*/ elephant this;  /* личные данные */
/*static*/ int thisState;         /* состояние */

main(int an, char *av[])
{
	char msg[40];
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

	/* завершение процесса */
	exit(0);
}
