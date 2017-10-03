/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void perror() {
	switch (errno) {
		case EFAULT:
			write(1,"Invalid address         \n",20);
			break;
		case EINVAL:
			write(1,"Invalid argument        \n",20);
			break;
		default:
			write(1,"Invalid error code      \n",20);
	}
}

int write(int fd, char *buffer, int size){
  int ret = 0;
  __asm__ __volatile__ (
    // "movl %1, %%ebx;"
    // "movl %2, %%ecx;"
    // "movl %3, %%edx;"
    "movl $4, %%eax;"
    "int $0x80;"
    : "=r"(ret) //operand = for write only => pone el valor output en ret
    : "b"(fd), "c"(buffer), "d"(size) //b for ebx, c for ecx, d for edx
    : "%eax"
  );
  //TODO: ebx, ecx , edx no se toquen durante la ejecucion
  if (ret < 0) {
  	errno = -ret;
  	return -1;
  }
  errno = 0;
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

