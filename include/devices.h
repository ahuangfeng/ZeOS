#ifndef DEVICES_H__
#define  DEVICES_H__

#define BUFF_SIZE 20
#define HEAD 1
#define TAIL 0

#include <sched.h>

int sys_write_console(char *buffer,int size);

struct list_head keyboardqueue;
struct circular_buff {
  char buff[BUFF_SIZE];
  int size;
};

struct circular_buff global_buff;

void cb_init(struct circular_buff *cb);
void cb_push(struct circular_buff * cb, char c);
char cb_pop(struct circular_buff * cb);

void block(struct task_struct * process, int head);
void unblock();

#endif /* DEVICES_H__*/
