/*
 * libc.c
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void perror()
{
	switch (errno)
	{
	case 0:
		write(1, "\nNo error\n", 10);
		break;
	case EFAULT: //14
		write(1, "\nInvalid address\n", 17);
		break;
	case EINVAL: //22
		write(1, "\nInvalid argument\n", 18);
		break;
	case ENOSYS:
		write(1, "\nNot implemented\n", 17);
		break;
	default:
		write(1, "\nInvalid error code\n", 20);
	}
}

int write(int fd, char *buffer, int size)
{
	int ret = 0;
	__asm__ __volatile__(
		// "movl %1, %%ebx;" // ponemos el parametro fd en ebx
		// "movl %2, %%ecx;" // ponemos el parametro buffer en ecx
		// "movl %3, %%edx;" // ponemos el parametro size en edx
		"movl $4, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		: "=r"(ret)						  //operand = for write only => pone el valor output en ret
		: "b"(fd), "c"(buffer), "d"(size) //b for ebx, c for ecx, d for edx
		:);
	if (ret < 0)
	{
		errno = -ret;
		return -1;
	}
	errno = 0;
	return ret;
}

int read(int fd, char *buf, int count){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(5), "b"(fd), "c"(buf), "d"(count)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int getpid(void)
{
	int ret;
	__asm__ __volatile__(
		// "movl $20, %%eax;"
		"int $0x80;"
		// "movl %%eax, %0;" //moving eax to the first variable--> in this case ret
		: "=a"(ret) // movl eax to ret
		: "a"(20)   //ponemos el 20 en eax
		:
	);
	return ret;
}

int fork(void)
{
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(2) // 2 in eax
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int get_stats(int pid, struct stats *st){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(35), "b"(pid), "c"(st)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int clone(void (*function) (void), void *stack){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(19), "b"(function), "c"(stack)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

void exit(void) {
	// write(1,"-->Exit\n",8);
	__asm__ volatile(
		"int $0x80;"
		:
		: "a"(1) // 2 in eax
		:);

}

int sem_init(int n_sem, unsigned int value){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(21), "b"(n_sem), "c"(value)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int sem_wait(int n_sem){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(22), "b"(n_sem)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int sem_signal(int n_sem){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(23), "b"(n_sem)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

int sem_destroy(int n_sem){
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(24), "b"(n_sem)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}

long long int gettime()
{
	long long int result = 0;
	__asm__ __volatile__(
		"movl $10, %%eax;"
		"int $0x80;"
		: "=r"(result)
		:
		: "%eax");
	return result;
}

void writeNumber(char * message, int a ){
	char res[10];
	itoa(a,res);
	write(1,message,strlen(message));
	write(1,res,10);
}

void itoa(int a, char *b)
{
	int i, i1;
	char c;

	if (a == 0)
	{
		b[0] = '0';
		b[1] = 0;
		return;
	}

	i = 0;
	while (a > 0)
	{
		b[i] = (a % 10) + '0';
		a = a / 10;
		i++;
	}

	for (i1 = 0; i1 < i / 2; i1++)
	{
		c = b[i1];
		b[i1] = b[i - i1 - 1];
		b[i - i1 - 1] = c;
	}
	b[i] = 0;
}

int strlen(char *a)
{
	int i;

	i = 0;

	while (a[i] != 0)
		i++;

	return i;
}

void *sbrk(int increment) {
	volatile int ret;
	__asm__ volatile(
		"int $0x80;"
		: "=a"(ret) //output in ret
		: "a"(6), "b"(increment)
		:);
	if (ret >= 0){
		return ret;
	}else{
		errno = -ret;
		return -1;
	}
}
