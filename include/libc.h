/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();
void writeNumber(char * message, int a );

void exit();
void perror();
int get_stats(int pid, struct stats *st);
int clone(void (*function) (void), void *stack);
int sem_init(int n_sem, unsigned int value);
int sem_wait(int n_sem);
int sem_signal(int n_sem);
int sem_destroy(int n_sem);
long long int gettime();


#endif  /* __LIBC_H__ */
