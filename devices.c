#include <io.h>
#include <utils.h>
#include <list.h>
#include <devices.h>
#include <system.h>


// Queue for blocked processes in I/O 
struct list_head keyboardqueue;

struct circular_buff global_buff;

void cb_init(struct circular_buff *cb){
  cb->size = 0;
}

int cb_isEmpty(struct circular_buff *cb){
  if(cb->size == 0){
    return 1;
  }else{
    return 0;
  }
}

void cb_push(struct circular_buff * cb, char c){
  if(cb->size < BUFF_SIZE){
    cb->buff[cb->size] = c;
    cb->size++;
  }
}

int cb_isFull(struct circular_buff * cb){
  if(cb->size == BUFF_SIZE){
    return 1;
  }else{
    return 0;
  }
}

int cb_hasEnough(struct circular_buff * cb, int nb){
  if(cb->size >= nb){
    return 1;
  }else{
    return 0;
  }
}

char cb_pop(struct circular_buff * cb){
  if(cb->size > 0){
    //TODO: y que devuelve aqui?
  }else {
    cb->size-=1;
    return cb->buff[cb->size];
  }
}

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

void block(struct task_struct * process, int head){
  // process->read_count = read_count;
  // char bu[10];
  // itoa(process->read_count,bu);
  // printk(bu);
  // printk("-");
  // char bu2[10];
  // itoa(read_count,bu2);
  // printk(bu2);
  // printk("F");
  if(head == 1){
    list_add(process,&keyboardqueue);
  }else {
    list_add_tail(process,&keyboardqueue);
  }
}

void unblock(){
  struct list_head* l_head = list_first(&keyboardqueue);
  list_del(l_head);
  list_add(l_head,&readyqueue);
  sched_next_rr();
}