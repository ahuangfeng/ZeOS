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

int sys_fork()
{
  int PID=-1;
  int frames[NUM_PAG_DATA];  
  
  if(list_empty(&freequeue)) return -ENOMEM; //out of memory
  
  struct list_head *new_listPointer = list_first(&freequeue);
  list_del(new_listPointer);

  struct task_struct * newPCB = list_head_to_task_struct(new_listPointer);
  struct task_struct * currentPCB = current();
  // modificar taula de pagines per a que poguem arribar a les pagines del usuari!


  
  //Inicializacion de paginas en padre
  int pag;
  int new_ph_pag;
  page_table_entry * pare_PT =  get_PT(currentPCB); //pare
  page_table_entry * fill_PT = get_PT(newPCB); //fill
  page_table_entry * dir_current = get_DIR(currentPCB);

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
  
  copy_data(currentPCB, newPCB, 4096); //4096 bytes
  
  allocate_DIR(newPCB);

  /* CODE User */
  for (pag=0;pag<NUM_PAG_CODE;pag++){
    fill_PT[PAG_LOG_INIT_CODE+pag].entry = pare_PT[PAG_LOG_INIT_CODE+pag].entry;
  }

  /* DATA User */ 
  for (pag=0;pag<NUM_PAG_DATA;pag++){
    set_ss_pag(fill_PT,PAG_LOG_INIT_DATA+pag,frames[pag]);
  }

  int adress_disponible = PAG_LOG_INIT_DATA + NUM_PAG_DATA;
  for(pag = 0 ; pag <NUM_PAG_DATA; pag++ ){
    set_ss_pag(pare_PT,adress_disponible+pag,frames[pag]);
    copy_data((void *) ((PAG_LOG_INIT_DATA+pag) << 12),(void *) ((adress_disponible+pag)<<12),PAGE_SIZE);
    del_ss_pag(pare_PT,adress_disponible+pag);
  }
  set_cr3(dir_current);

  PID = ultimPID;
  ultimPID++;
  newPCB->PID = PID;
  // newPCB->state = ST_READY;

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
