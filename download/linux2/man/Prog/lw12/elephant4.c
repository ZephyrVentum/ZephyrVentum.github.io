/**********************************************/
/* Пример для работы #4                       */
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
#include <fcntl.h>
#include <sys/resource.h>

#include "../common/wait.h"
#include "../common/curtime.h"
#include "../common/elephant.h"

/*static*/ elephant this; /* описание данного Слона */
/*static*/ int thisState;        /* состояние Слона */
/*static*/ int fdMy;  /* входной канал Слона O_o*/
/*static*/ int fdG;   /* выходной канал Слона o_O*/

/**********************************************/
main(int an, char *av[])
{
	long need, port; /* потребность и полученная порция */

	/* прием параметров */
	strcpy(this.name,av[0]);
	sscanf(av[1],"%d",&(this.age)); 
	sscanf(av[2],"%lf",&(this.weight));

	/* инициализация генератора случайных чисел */  
	srand(time(NULL));
	/* состояние - 0 */
	thisState=0;

	printf("%s Слон %s приступил\n",curtime(),this.name);
	/* открытие входного канала на чтение 
	(при этом Слон ждет, когда Ганеша откроет этот канал для записи) */
	fdMy=open(this.name,O_RDONLY);
	/* вычисление потребности */
	need=this.weight*500;
	/* цикл, пока потребность не будет удовлетворена */
	while (need>0)
	{
		/* ожидание и чтение размера очередной порции */
		read(fdMy,&port,sizeof(long));
		printf("%s Слон %s получил %ld\n",
		curtime(),this.name,port);
		/* потребление порции */
		a0wait((int)(port/this.age*10));
		/* уменьшение потребности */
		need-=port;
		printf("%s Слон %s выпил %ld, остаток потребности - %ld\n",  
		curtime(),this.name,port,need);
	}
	close(fdMy);

	/* открытие выходного канала на запись 
	(при этом Слон ждет, когда Ганеша откроет этот канал для чтения) */
	fdG=open("Ganesha",O_WRONLY);
	/* вывод доклада в канал и завершение */
	write(fdG,this.name,strlen(this.name)+1);
	close(fdG); 
	exit(0);
}
