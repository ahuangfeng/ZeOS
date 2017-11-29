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

void test_write(){
	if(write(1, " \nHOLA que tal ?", 16) == -1) perror();
	volatile int i = 0;
	while(i<2000){
		i++;
	}
}

void test_gettime(){
	volatile int o = gettime();
	char * str = "";
	if(o > 0){
		if(write(1,"gettime->",9)) perror();
		itoa(o,str);
		if(write(1,str,strlen(str))) perror();
		write(1,"\n",1);
	}
}

void test_pid(){
	//test pid
	volatile int pid = getpid();
	char * buffer = "";
	itoa(pid,buffer);
	write(1,"pid of first task --> ",22);
	if(write(1,buffer,strlen(buffer))) perror();
}

void test_fork(){
	volatile int pid = fork();
	if(pid == 0){

		exit();
		// char resString[20];
		// itoa(pid,resString);
		// write(1,"\nhijo:",6);
		// write(1,resString,strlen(resString));
	}else if(pid > 0){
		//funciona
		// exit();
		char resString[20];
		itoa(pid,resString);
		write(1,"\nPadre --> pid del hijo:",24);
		write(1,resString,strlen(resString));
		struct stats *St;
		get_stats(1,St);
		// St->remaining_ticks = 20;
		// writeNumber("UserTicks:",St->remaining_ticks);
	}else{
		write(1,"Error!",6);
	}
}

void functionForClone(){
	write(1,"Hola!!",6);
	exit();
}

void test_clone(){
	char pila[500];
	clone(functionForClone,&pila);
}

void test_sem(){
	sem_init(1, 1);
	int pid = fork();
	if(pid == 0){
		int err = sem_destroy(1);
		perror();
		writeNumber("error:",err);
		write(1,"hijo",4);
	}else{
		int err = sem_destroy(1);
		perror();
		writeNumber("error:",err);
		write(1,"Padre",5);
	}
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{

	// test_write();
	//test pid
	// test_pid();

	//test fork
	// test_fork();

	// runjp_rank(14,16);
	runjp();
	write(1,"hola",4);
	// test_sem();
	// test_clone();
	// exit();
	while(1){
		//nothing
	}

  	return 0;
}
