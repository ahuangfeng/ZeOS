#include <io.h>
#include <utils.h>
#include <list.h>
#include <devices.h>


// Queue for blocked processes in I/O 
struct list_head keyboardqueue;

struct circular_buff global_buff;

void cb_init(struct circular_buff *cb){
  cb->size = 0;
}

void cb_push(struct circular_buff * cb, char c){
  if(cb->size == BUFF_SIZE){

  }else{
    cb->buff[cb->size] = c;
    cb->size++;
  }
}

char cb_pop(struct circular_buff * cb){
  if(cb->size == 0){

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
  if(head == 1){
    list_add(process,&keyboardqueue);
  }else {
    list_add_tail(process,&keyboardqueue);
  }
}

void unblock(){
  struct list_head* l_head = list_first(&keyboardqueue);
  list_del(l_head);
  list_add_tail(l_head,&readyqueue);
}