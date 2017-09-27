#include <libc.h>

char buff[24];

int pid;


int add( int par1, int par2){
	__asm__ __volatile__ ( 
                "movl 0x8(%ebp), %eax;"
                "addl 0xc(%ebp), %eax;"
                //"add %edx, %eax;"
    );
    //return par1+par2;
}

long inner(long n){
	int i;
	long suma;
	suma = 0;
	for( i=0; i<n; i++){
		suma = add(suma,i);
		//suma = suma +i;
	}
	return suma;
}

long outer(long n){
	int i;
	long acum;
	acum = 0;
	for(i=0; i<n;i++){
		int k = inner(i);
		acum = add(acum,k);//acum + inner(i);
	}
	return acum;
}



int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	//long count = inner(4);

    long count, acum;
    count = 75;
    acum = 0;
    acum = outer(count);

/*    __asm__ __volatile__ ( "push %ebp"
                "mov %esp, %ebp;"
                "mov 0x8(%ebp), %edx;"
                "mov 0xc(%ebp), %eax;"
                "add %edx, %eax;"
                "pop %ebp;"
                "ret;"
                "nop;"
    );*/

	/*int a,b;
	a = 2;
	b = 3;
	 add(a,b);
*/

  while(1) { }
  	return 0;
}
