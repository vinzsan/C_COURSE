#define NULL ((void *)0)
#define EOF -1
#define MAP_ANONYMOUS 0x20
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_SHARED 0x01
#define PROT_WRITE 0x02
#define PROT_READ 0x01
#define PROT_EXEC 0x04
#define PROT_NONE 0x00
#define MAP_FAILED ((void *)-1)
#define EQUATE -1
#define SA_RESTART 0x10000000
#define SIGINT 2
#define SA_RESTORE 0x04000000

typedef unsigned long size_t;
typedef unsigned long sigset_t[16];

struct sigaction{
	void (*sa_handler)(int);	
	unsigned long sa_flags;
	void (*sa_restorer)(void);
	sigset_t sa_mask;
};

typedef struct
{
	void *(*Mapping)(void *ptr,size_t size,int PROT,int MAP,int FD,int FLAGS);
	int (*MappFree)(void *ptr,size_t size);
} Memory;


void *VirtualMap(void *ptr,size_t size,int PROT,int MAP,int FD,int FLAGS){
	void *retptr;
	__asm__ __volatile__(
		"movq $9,%%rax\n\t"
		"movq %1,%%rdi\n\t"
		"movq %2,%%rsi\n\t"
		"movq %3,%%rdx\n\t"
		"movq %4,%%r10\n\t"
		"movq %5,%%r8\n\t"
		"movq %6,%%r9\n\t"
		"syscall\n\t"
		"movq %%rax,%0"
		: "=r"(retptr)
		: "r"(ptr), "r"(size), "r"((long)PROT), "r"((long)MAP), "r"((long)FD), "r"((long)FLAGS)
		: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9");
	return retptr;
}

int VirtualFree(void *ptr,size_t size){
  if(!ptr) return -1;
  __asm__ __volatile__(
		       "movq $11,%%rax\n\t"
		       "movq %0,%%rdi\n\t"
		       "syscall"
		       :
		       :"r"(ptr),"r"(size)
		       :"rax","rdi"
		       );
  return 0;
}

char *strcpy(char *dest,char *src){
  if(!dest || !src) return NULL;
  char *d = dest;
  while((*d++ = *src++));
  return dest;
}

size_t strlen(char *str){
	if(!str) return 0;
	size_t size = 0;
	while((size[str])) size++;
	return size;
}

/*This is used for print,just print char*/
int print(char *str,size_t length){
  if(length >= 0){
    while((length[str])) length++;
  }
  __asm__ __volatile__(
		       "movq $1,%%rax\n\t"
		       "movq $1,%%rdi\n\t"
		       "movq %0,%%rsi\n\t"
		       "movq %1,%%rdx\n\t"
		       "syscall"
		       :
		       :"r"(str),"r"(length)
		       :"rax","rdi","rsi","rdx"
		       );
  return (int)length;
}

long rt_sigaction(int SIGNAL,struct sigaction *act,struct sigaction *old,size_t set){
	long result;
	__asm__ __volatile__(
		"movq $13,%%rax\n\t"
		"movq %1,%%rdi\n\t"
		"movq %2,%%rsi\n\t"
		"movq %3,%%rdx\n\t"
		"movq %4,%%r10\n\t"
		"syscall\n\t"
		"movq %%rax,%0"
		:"=r"(result)
		:"r"((long)SIGNAL),"r"(act),"r"(old),"r"(set)
		:"rax","rdi","rsi","rdx","r10"
	);
	return result;
}

void restorer(){
	__asm__ __volatile__("movq $15,%%rax\n\tsyscall":::"rax");
}

int exit_(int echo){
	__asm__(
		"movq $60,%%rax\n\t"
		"movq %0,%%rdi\n\t"
		"syscall"
		:
		:"r"((long)echo)
		:"rax","rdi"
	);
	return 0;
}

static char* author(){
	print("i am a..\n",EQUATE);
	return "femboys :3";
}

int println(char *buffer, size_t len) {
    if (!buffer || len == 0) return -1;
	return (int)print(buffer, len);
}

void handler(int sig){
	println("Signal recieved\n", 17);
	exit_(0);
}

int _start(){
	struct sigaction signal = {0};
	signal.sa_handler = handler;
	signal.sa_restorer = restorer;
	for (int a = 0; a < 16; a++)
	{
		signal.sa_mask[a] = 0;
	}
		signal.sa_flags = SA_RESTART | SA_RESTORE;
	rt_sigaction(2, &signal, NULL,sizeof(long));
	while(1){
		asm("pause");
	}
exit:
	asm(
		"movq $60,%%rax\n\t"
		"xorq %%rdi,%%rdi\n\t"
		"syscall"
		:	
		:
		: "rax", "rdi");
}
