/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <declaracions.h>

#include <libc.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define TAMANYBUFF 4

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork(){
  return 0;
}

int sys_fork()
{
  int PID=-1;
  //a
  if(list_empty(&freequeue)) return -ENOMEM; //out of memory
  
  struct list_head *new_listPointer = list_first(&freequeue);
  list_del(new_listPointer);
  
  struct task_struct * newPCB = list_head_to_task_struct(new_listPointer);
  struct task_struct * currentPCB = current();
  // modificar taula de pagines per a que poguem arribar a les pagines del usuari!
  
  //b
  copy_data(currentPCB, newPCB, 4096); //4096 bytes
  //c
  allocate_DIR(newPCB);
  
  //d
  //Inicializacion de paginas en padre
  int frames[NUM_PAG_DATA];  
  int pag;
  int new_ph_pag;
  // printk("0");
  page_table_entry * pare_PT =  get_PT(currentPCB); //pare
  page_table_entry * dir_current = get_DIR(currentPCB);
  // printk("1");
  /* frames for DATA */
  for (pag=0;pag<NUM_PAG_DATA;pag++){
    new_ph_pag=alloc_frame();
    if(new_ph_pag == -1){
      //si error, buidem pagines que em agafat
      while(pag != 0){
        free_frame(frames[--pag]);
      }
      return -ENOMEM;
    }else{
      frames[pag] = new_ph_pag;
    } 
  }
  // printk("2");
  
  
  // printk("3");
  page_table_entry * fill_PT = get_PT(newPCB); //fill
  // printk("4");
  
  /* Kernel Data and code*/
  //e-A
  for(pag = 0; pag < NUM_PAG_KERNEL; pag++){
    set_ss_pag(fill_PT,pag, get_frame(pare_PT,pag));
  } 
  
  /* CODE User */ 
  for (pag=0;pag<NUM_PAG_CODE;pag++){
    set_ss_pag(fill_PT,PAG_LOG_INIT_CODE+pag,get_frame(pare_PT,PAG_LOG_INIT_CODE+pag));
  }
  
  // printk("6");
  /* DATA User */ 
  //e1-B
  for (pag=0;pag<NUM_PAG_DATA;pag++){
    set_ss_pag(fill_PT,PAG_LOG_INIT_DATA+pag,frames[pag]);
  }
  
  // printk("7");
  // establecer espacio en padre para despues linkear a hijo
  //e-2
  int adress_disponible = PAG_LOG_INIT_DATA + NUM_PAG_DATA;
  for(pag = 0 ; pag <NUM_PAG_DATA; pag++ ){
    set_ss_pag(pare_PT,adress_disponible+pag,frames[pag]); //PAGE_SIZE = 0x1000 = 1000 0000 0000
    copy_data((void *) ((PAG_LOG_INIT_DATA+pag) << 12),(void *) ((adress_disponible+pag)<<12),PAGE_SIZE);
    del_ss_pag(pare_PT,adress_disponible+pag);
  }
  set_cr3(dir_current);

  //f
  PID = ultimPID;
  ultimPID++;
  newPCB->PID = PID;
  
  // printk("10");
  // printk(";11");
  //h
  union task_union * tku_fill = (union task_union*) newPCB;
  // 5 push de hardware i 11 de SAVE_ALL + @handler --> ponemos en -18
  tku_fill->stack[KERNEL_STACK_SIZE-18] = (unsigned long)&ret_from_fork;
  // printk(";12");
  tku_fill->stack[KERNEL_STACK_SIZE-19] = 0;
  // printk(";13");
  tku_fill->task.proces_esp = (unsigned long*)&(tku_fill->stack[KERNEL_STACK_SIZE-19]);
  
  //i
  list_add_tail(&(newPCB->list),&readyqueue);
  
  
  return PID;
}



void sys_exit()
{  
}

int sys_write(int fd, char * buffer, int size) {
  /*fd: file descriptor. In this delivery it must always be 1.
    buffer: pointer to the bytes.
    size: number of bytes.
    return ’ Negative number in case of error (specifying the kind of error) and
    the number of bytes written if OK.
  */

  char buff[TAMANYBUFF];

  //checking the parameters
  int check = check_fd(fd,ESCRIPTURA);
  if(check != 0){
    return check;
  }
  if(buffer == NULL){
    return -EFAULT; // Bad Adress
  }
  if(size < 0){
    return -EINVAL; // Invalid Argument
  }
  
  int res = 0;
  while(size>TAMANYBUFF){  // TAMANYBUFF = 4
    copy_from_user(buffer, buff, TAMANYBUFF);
    sys_write_console(buff, TAMANYBUFF);
    size -= TAMANYBUFF;
    buffer += TAMANYBUFF;
  }
  if(size>0){
    copy_from_user(buffer, buff, size);
    sys_write_console(buff,size);
  }
  return res;

}

int sys_gettime(){
  return zeos_ticks;
}
