/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <libc.h>

#include <zeos_interrupt.h>

#include <declaracions.h>
#include <mm.h>
#include <devices.h>
#include <system.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

// Declaracions:
// void keyboard_handler();
// void clock_handler();
// void system_call_handler();


char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(32,clock_handler,0);
  setInterruptHandler(33,keyboard_handler,0);
  setTrapHandler(0x80, system_call_handler, 3);

  setInterruptHandler(14, page_fault_handler_, 0);

  set_idt_reg(&idtR);
}

void page_fault_routine_bis(){
  // // int pageFault = tss.esp0;
  // struct task_struct* ts = current();
  // // union task_union* tu = (union task_union*) ts;
  // // int nbError = tu->stack[KERNEL_STACK_SIZE-5];
  // int nbError = ts->proces_esp;
  // // int stk = tu->stack[pageFault];
  // char ss[100];
  // itoa(nbError,ss);
  printk("Page fault at ");
  // printk(ss);
}

void userToSystem_routine(){
  unsigned long current_ticks = get_ticks();
  current()->stadisticas.user_ticks += current_ticks-(current()->stadisticas.elapsed_total_ticks);
  current()->stadisticas.elapsed_total_ticks = current_ticks;
}

void systemToUser_routine(){
  unsigned long current_ticks = get_ticks();
  current()->stadisticas.system_ticks += current_ticks-(current()->stadisticas.elapsed_total_ticks);
  current()->stadisticas.elapsed_total_ticks = current_ticks;
}


void keyboard_routine(){
  unsigned char num = inb(0x60);
  char mask = 0x80;
  char pressed = num & mask;
  pressed = pressed >> 7;
  if(pressed){
    num = num & 0x7F;
    int valNum = (int) num;
    char c = 'C';
    if(valNum < 98){
      c = char_map[valNum];
    }
    if( c == '\0'){
      c = 'C';
    }
    printc_xy(0,0,c);
    // char buf[BUFF_SIZE];
    // itoa(global_buff.size,buf);
    // printk(buf);
    cb_push(&global_buff, c);
    if(!list_empty(&keyboardqueue)){
      // struct list_head * lh = list_first(&keyboardqueue);
      // struct task_struct *tsk = list_head_to_task_struct(lh);

      //  int abb = read_count;
      //  int baa = global_buff.size;
      // char buf[BUFF_SIZE];
      // itoa(tsk->read_count,buf);
      // printk(buf);
      // printk("-");
      // char buf2[BUFF_SIZE];
      // itoa(global_buff.size,buf2);
      // printk(buf2);
      if((cb_isFull(&global_buff)) || (read_count <= global_buff.size)){
        unblock();
        // printk("Unblock");
      }
    }
  }
}

void clock_routine(){
  ++zeos_ticks;
  zeos_show_clock();
  schedule();
}
