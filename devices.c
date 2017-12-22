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
  cb->input = -1;
}

// int cb_isEmpty(struct circular_buff *cb){
//   if(cb->size == 0){
//     return 1;
//   }else{
//     return 0;
//   }
// }

void cb_push(struct circular_buff * cb, char c){
  cb->input++;
  cb->input = cb->input%BUFF_SIZE;
  cb->buff[cb->input] = c;
  if (cb->size < BUFF_SIZE) cb->size++;
}

int cb_isFull(struct circular_buff * cb){
  if(cb->size == BUFF_SIZE){
    return 1;
  }else{
    return 0;
  }
}

// int cb_hasEnough(struct circular_buff * cb, int nb){
//   if(cb->size >= nb){
//     return 1;
//   }else{
//     return 0;
//   }
// }

char* cb_getChars(struct circular_buff * cb, int nb_char){
  char array[nb_char];
  int i = 0;
  while(i<nb_char){
    array[i] = cb->buff[cb->output];
    cb->output++;
    cb->output = cb->output%BUFF_SIZE;
    i++;
  }
  cb->size = cb->size - nb_char;
  return array;
}

char cb_getChar(struct circular_buff *cb){
  cb->output++;
  cb->output = cb->output%BUFF_SIZE;
  return cb->buff[cb->output-1];
}

// void cb_copy(struct circular_buff * cb, char * buff,int tmp_count){
//   copy_to_user(cb->buff[cb->output], buff, tmp_count);
// }

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

void block(struct list_head * process_head, int head){
  // process_head->read_count = read_count;
  // char bu[10];
  // itoa(process_head->read_count,bu);
  // printk(bu);
  // printk("-");
  // char bu2[10];
  // itoa(read_count,bu2);
  // printk(bu2);
  // printk("F");
  if(head == 1){
    list_add(process_head,&keyboardqueue);
  }else {
    list_add_tail(process_head,&keyboardqueue);
  }
}

void unblock(){
  struct list_head* l_head = list_first(&keyboardqueue);
  struct task_struct * tsk = list_head_to_task_struct(l_head);
  tsk->state = ST_READY;
  list_del(l_head);
  list_add(l_head,&readyqueue);
  sched_next_rr();
}