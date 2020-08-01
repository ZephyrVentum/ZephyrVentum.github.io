#ifndef _elephantH_
#define _elephantH_

#define MAX_NAME_LENGTH 32
#define NE 8 // количество Слонов в стаде

/* Структура, описыавающая Слона */
typedef struct _elephant_
{
	char name[MAX_NAME_LENGTH]; // имя
	int age;        // возраст (полных лет)
	double weight;  // вес (в тоннах)
} elephant;

/* Структура, описывающая Слона в мониторе: содержит личные данные Слона и управляющую информацию о нем */
typedef struct _meleph_
{
	elephant *el; // указатель на личные данные
  	pid_t chpid;         // ID процесса
  	int prty;            // приоритет процесса
  	char status;         // состояние процесса
} meleph;

/* личные данные Слонов */
elephant ee[NE] = 
{
	{"Tandy",  3, 1.7 },
	{"Aun",    4, 2.2 },
	{"Assam", 40, 5.8 },
	{"Maya",  37, 4.3 },
	{"BakZap",52, 4.5 },
	{"Hao",   14, 3.2 },
	{"Hathy", 42, 6.9 },
	{"Kitty", 10, 3 }
};

#endif
