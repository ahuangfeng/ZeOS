/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void perror() {
	switch (errno) {
		case 0:
			write(1,"\nNo error\n",10);
			break;
		case EFAULT: //14
			write(1,"\nInvalid address\n",17);
			break;
		case EINVAL: //22
			write(1,"\nInvalid argument\n",18);
			break;
		case ENOSYS:
			write(1,"\nNot implemented\n",17);
			break;
		default:
			write(1,"\nInvalid error code\n",20);
	}
}

int write(int fd, char *buffer, int size){
	int ret = 0; //TODO: lo ponemos a -1?
	__asm__ __volatile__ (
		// "movl %1, %%ebx;" // ponemos el parametro fd en ebx
		// "movl %2, %%ecx;" // ponemos el parametro buffer en ecx
		// "movl %3, %%edx;" // ponemos el parametro size en edx
		"movl $4, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;"
		: "=r"(ret) //operand = for write only => pone el valor output en ret
		: "b"(fd), "c"(buffer), "d"(size) //b for ebx, c for ecx, d for edx
		: 
	);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	errno = 0;
	return ret;
}

int getpid(void){
	//TODO: hay que implementarlo!
	
	int ret = -1;
	__asm__ __volatile__(
		"movl $20, %%eax;"
		"int $0x80;"
		"movl %%eax, %0;" //moving eax to the first variable--> in this case ret
		: "=g"(ret)
		:
		: "ax"
	);
	// if (ret < 0) {
	// 	errno = -ret;
	// 	return -1;
	// }
	// errno = 0;
	return ret;
}

long long int gettime(){
	long long int result = 0;
	__asm__ __volatile__(
		"movl $10, %%eax;"
		"int $0x80;"
		: "=r"(result)
		:
		: "%eax"
	);
	return result;
}

void itoa(int a, char *b)
{
	int i, i1;
	char c;
	
	if (a==0) { b[0]='0'; b[1]=0; return ;}
	
	i=0;
	while (a>0)
	{
		b[i]=(a%10)+'0';
		a=a/10;
		i++;
	}
	
	for (i1=0; i1<i/2; i1++)
	{
		c=b[i1];
		b[i1]=b[i-i1-1];
		b[i-i1-1]=c;
	}
	b[i]=0;
}

int strlen(char *a)
{
	int i;
	
	i=0;
	
	while (a[i]!=0) i++;
	
	return i;
}

