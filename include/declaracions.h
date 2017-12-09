// Totes les declaracions que hem de fer 
struct task_struct;
void keyboard_handler();
void clock_handler();
void system_call_handler();

void page_fault_handler_();

long long int zeos_ticks;
int fork();
unsigned long get_ticks();
void init_stats(struct task_struct *current);
int calculateDIR_ID(struct task_struct *t);
int sys_sem_destroy(int n_sem);
void zeos_init_auxjp();
void runjp();
