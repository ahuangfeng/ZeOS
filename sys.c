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

extern int quantum_restant;

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
    if(new_ph_pag < 0){
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
  newPCB->quantum = currentPCB->quantum;

  //h
  union task_union * tku_fill = (union task_union*) newPCB;
  // 5 push de hardware i 11 de SAVE_ALL + @handler --> ponemos en -18
  tku_fill->stack[KERNEL_STACK_SIZE-18] = (unsigned long)&ret_from_fork;
  tku_fill->stack[KERNEL_STACK_SIZE-19] = 0;
  tku_fill->task.proces_esp = (unsigned long*)&(tku_fill->stack[KERNEL_STACK_SIZE-19]);
  newPCB->state = ST_READY;
  //i
  list_add_tail(&(newPCB->list),&readyqueue);

  //reinicia las stats del hijo
  init_stats(newPCB);
  // int i = currentPCB->stadisticas.elapsed_total_ticks;

  return PID;
}

void init_stats(struct task_struct *current){
  current->stadisticas.blocked_ticks = 0;
  current->stadisticas.elapsed_total_ticks = get_ticks();
  current->stadisticas.ready_ticks = 0;
  current->stadisticas.remaining_ticks = get_quantum(current); //get_ticks();
  current->stadisticas.system_ticks = 0;
  current->stadisticas.total_trans = 0;
  current->stadisticas.user_ticks = 0;
}

void sys_clone(void (*function) (void), void *stack){
  int PID=-1;

  if(!access_ok(0,function,sizeof(void))) return -EFAULT;
  if(!access_ok(0,stack,sizeof(void))) return -EFAULT;
  // if(function == NULL) return -1;

  //a
  if(list_empty(&freequeue)) return -ENOMEM; //out of memory

  struct list_head *new_listPointer = list_first(&freequeue);
  list_del(new_listPointer);

  struct task_struct * newPCB = list_head_to_task_struct(new_listPointer);
  struct task_struct * currentPCB = current();
  union task_union * tku_fill = (union task_union*) newPCB;

  //b
  copy_data(currentPCB, newPCB, sizeof(union task_union)); //4096 bytes

  //c
  // allocate_DIR(newPCB);

  int pos = calculateDIR_ID(currentPCB);
  directories_refs[pos]++;
  // newPCB->dir_pages_baseAddr = &dir_pages[pos];

  //d
  //Inicializacion de paginas en padre
  // page_table_entry * dir_current = get_DIR(currentPCB);
  // set_cr3(dir_current);

  //f
  PID = ultimPID;
  ultimPID++;
  newPCB->PID = PID;
  newPCB->quantum = 10;

  //h
  // 5 push de hardware i 11 de SAVE_ALL + @handler --> ponemos en -18
  tku_fill->stack[KERNEL_STACK_SIZE-2] = (unsigned long)stack;
  tku_fill->stack[KERNEL_STACK_SIZE-5] = (unsigned long)function;
  tku_fill->stack[KERNEL_STACK_SIZE-18] = &ret_from_fork;
  tku_fill->stack[KERNEL_STACK_SIZE-19] = 0;
  tku_fill->task.proces_esp = (unsigned long*)&(tku_fill->stack[KERNEL_STACK_SIZE-19]);
  newPCB->state = ST_READY;
  //i
  list_add_tail(&(newPCB->list),&readyqueue);

  //reinicia las stats del hijo
  init_stats(newPCB);
  // int i = currentPCB->stadisticas.elapsed_total_ticks;

  return PID;
}

void sys_exit()
{
  page_table_entry *pt_current = get_PT(current());

  for( int i = 0; i<NR_SEMAPHORES; i++){
    if(semaphore_list[i].pidOwner == current()->PID){
      sys_sem_destroy(i);
    }
  }

  int pos = calculateDIR_ID(current());
  directories_refs[pos]--;
  if(directories_refs[pos] == 0){
    //data user
    for (int i = 0; i < NUM_PAG_DATA; i++) {
      free_frame(get_frame(pt_current, PAG_LOG_INIT_DATA+i));
      del_ss_pag(pt_current, PAG_LOG_INIT_DATA+i);
    }
  }
  list_add_tail(&current()->list, &freequeue);
  current()->PID = -1;
  sched_next_rr();
}

int sys_sem_init(int n_sem, unsigned int value){
  if(n_sem >= 20 || n_sem < 0) return -EINVAL;
  if(semaphore_list[n_sem].pidOwner >= 0) return -EBUSY;

  semaphore_list[n_sem].counter = value;
  semaphore_list[n_sem].pidOwner = current()->PID;
  INIT_LIST_HEAD(&semaphore_list[n_sem].blocked_queue);

  return 0;
}

int sys_sem_wait(int n_sem){
  if(n_sem >= 20 || n_sem < 0) return -EINVAL;
  if(semaphore_list[n_sem].pidOwner < 0) return -EINVAL;

  semaphore_list[n_sem].counter--;
  if(semaphore_list[n_sem].counter<0){
    list_add_tail(&current()->list,&semaphore_list[n_sem].blocked_queue);
    sched_next_rr();
  }

  if(semaphore_list[n_sem].pidOwner < 0) return -EINVAL;

  return 0;
}

int sys_sem_signal(int n_sem){
  if(n_sem >= 20 || n_sem < 0) return -EINVAL;
  if(semaphore_list[n_sem].pidOwner < 0) return -EINVAL;

  semaphore_list[n_sem].counter++;
  // !list_empty(&semaphore_list[n_sem].blocked_queue)
  if(semaphore_list[n_sem].counter<=0){
    struct list_head * e = list_first( &semaphore_list[n_sem].blocked_queue );
    list_del(e);
    list_add_tail(e,&readyqueue);
    struct task_struct* tks = list_head_to_task_struct(e);
    tks->state = ST_READY;
  }
  return 0;
}

int sys_sem_destroy(int n_sem){
  if(n_sem >= 20 || n_sem < 0) return -EINVAL;
  if(semaphore_list[n_sem].pidOwner < 0) return -EINVAL;

  int pidActual = current()->PID;
  if(pidActual != semaphore_list[n_sem].pidOwner) return -EPERM;

  if(!list_empty(&semaphore_list[n_sem].blocked_queue)){
    struct list_head * e;
    struct list_head * pos;
    list_for_each_safe(pos, e, &semaphore_list[n_sem].blocked_queue){
      list_del(pos);
      list_add_tail(pos,&readyqueue);
      struct task_struct* tks = list_head_to_task_struct(pos);
      tks->state = ST_READY;
    }
  }
  semaphore_list[n_sem].pidOwner = -1;
  return 0;
}

int sys_write(int fd, char * buffer, int size) {
  /*fd: file descriptor. In this delivery it must always be 1.
    buffer: pointer to the bytes.
    size: number of bytes.
    return ’ Negative number in case of error (specifying the kind of error) and
    the number of bytes written if OK.
  */
  int tamTotal = size;
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
  return tamTotal;

}

int sys_get_stats(int pid, struct stats *st){
  int ret = -1;
  if(pid < 0) return -EINVAL;
  if(!access_ok(VERIFY_WRITE,st,sizeof(struct stats))){
    return -EFAULT;
  }

  for( int i = 0; i<NR_TASKS ; i++){
    if(task[i].task.PID==pid){
      task[i].task.stadisticas.remaining_ticks = quantum_restant; // get_quantum(current());
      copy_to_user(&task[i].task.stadisticas,st,sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH;
}

int sys_gettime(){
  return zeos_ticks;
}
