/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

int ultimPID;
int quantum_restant = 0;
/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

struct task_struct *idle_task;

#if 1 
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1)
	{
		// printk("While infinit!\n");
	;
	}
}

void schedule(){
	update_sched_data_rr();
	if(needs_sched_rr()){
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}

void update_sched_data_rr(void){
	quantum_restant--;
}

int needs_sched_rr(void){
	if(quantum_restant == 0){
		if(!list_empty(&readyqueue)){
			return 1;
		}
		quantum_restant = get_quantum(current());
	}
	return 0;
}


void sched_next_rr(){
	struct task_struct * nou_tku;
	if(!list_empty(&readyqueue)){
		struct list_head * nou_list;
		nou_list = list_first(&readyqueue);
		nou_tku = list_head_to_task_struct(nou_list);
	}else{
		nou_tku = idle_task;
	}
	nou_tku->state = ST_RUN;
	quantum_restant = get_quantum(nou_tku);
	nou_tku->stadisticas.remaining_ticks = quantum_restant;
	
	unsigned long current_ticks = get_ticks();
	//actualitzar el old
	current()->stadisticas.system_ticks += current_ticks- (current()->stadisticas.elapsed_total_ticks);
	current()->stadisticas.elapsed_total_ticks = current_ticks;
	
	//actualitzar el new
	nou_tku->stadisticas.total_trans++;
	nou_tku->stadisticas.ready_ticks += current_ticks-(nou_tku->stadisticas.elapsed_total_ticks);
	nou_tku->stadisticas.elapsed_total_ticks = current_ticks;

	task_switch((union task_union *) nou_tku);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest){
	if(t->state != ST_RUN){
		list_del(&(t->list));
	}
	if(dest != NULL){ // el siguiente estado no es running
		list_add_tail(&(t->list),dest);
		if(dest == &readyqueue){
			t->state = ST_READY;
		}
	}else{
		t->state = ST_RUN;
	}
}

int get_quantum (struct task_struct *t){
	return t->quantum;
}

void set_quantum (struct task_struct * t, int new_quantum){
	t->quantum = new_quantum;
}

void init_idle (void)
{
	struct list_head * first_freequeue = list_first( &freequeue );
	struct task_struct * structura = list_head_to_task_struct(first_freequeue);
	list_del(first_freequeue); 
	// printk("\ninit_idle");
	// freequeue = *first_freequeue;
	
	structura->PID = ultimPID;
	ultimPID++;
	allocate_DIR(structura);
	
	union task_union * tku = (union task_union *) structura;
	tku->stack[KERNEL_STACK_SIZE-1] = ( unsigned long) &cpu_idle; 
	tku->stack[KERNEL_STACK_SIZE-2] = 0;
	structura->proces_esp = &(tku->stack[KERNEL_STACK_SIZE-2]);
	idle_task = structura;
	// init_stats(structura);
}

void task_switch(union task_union * new){
	// printk("task swiitch\n");
	__asm__ __volatile__(
		"pushl %%esi;"
		"pushl %%edi;"
		"pushl %%ebx;"
		// "pushl %%eax;" 
		"call inner_task_switch;"
		// "popl %%eax;"
		"popl %%ebx;"
		"popl %%edi;"
		"popl %%esi;"
		: /* no output */
		: "a"(new)
	);
	// printk("task swiitch2\n");
}

void inner_task_switch(union task_union * new){
	tss.esp0 = (DWord) &(new->stack[KERNEL_STACK_SIZE]);
	set_cr3(get_DIR((struct task_struct *) new));
	
	struct task_struct * old = current();
	struct task_struct * newtsk = (struct task_struct *) new;
	
	void * oldEsp = old->proces_esp;
	void * newEsp = newtsk->proces_esp;

	
	unsigned long * ebpAddres;
	__asm__ __volatile__(
		"movl %%ebp, %0;"
		: "=r"(ebpAddres)
		: 
	);
	old->proces_esp = ebpAddres;
	unsigned long * newEbpAddress = newtsk->proces_esp;
	__asm__ __volatile__(
		"movl %%ebp, %%esp;"
		"popl %%ebp;"
		"ret;"
		: 
		: "b"(newEbpAddress)
	);
}

void init_task1(void)
{
	struct list_head * first_freequeue = list_first( &freequeue );
	struct task_struct * mi_estructura = list_head_to_task_struct(first_freequeue);
	list_del(first_freequeue); 
	
	// printk("init_task1");
	union task_union * tku = (union task_union *) mi_estructura;
	
	mi_estructura->PID = ultimPID;
	ultimPID++;
	allocate_DIR(mi_estructura);
	set_user_pages(mi_estructura);
	
	tss.esp0 = (DWord) &(tku->stack[KERNEL_STACK_SIZE]);
	set_cr3(get_DIR(mi_estructura));
	// init_stats(mi_estructura);
}

void init_sched(){
	INIT_LIST_HEAD(&freequeue);

	for(int i = 0; i<NR_TASKS; i++){
		list_add_tail(&task[i].task.list,&freequeue);
		init_stats(&task[i].task);
	}
	//iniciar el quantum inicial
	quantum_restant = 10;
	INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

