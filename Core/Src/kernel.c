#include "kernel.h"
#include <stdio.h>
#include <stdbool.h>
#include "main.h"



extern UART_HandleTypeDef huart2;

uint32_t* MSP_INIT_VAL;
uint32_t* curr_sp;
uint32_t stack_count = 0;
uint32_t* stack_pool;
uint32_t stack_index = 0;
uint32_t TOTAL_STACK = 0x4000;


thread globalThread[9];

struct baseball{
	uint16_t base;
	uint16_t ball;
};


bool osCreateThread(void (*function_ptr)(void*), void* args){

	uint32_t* new_stackptr = New_Stack_Allocation(MSP_INIT_VAL);
	if(new_stackptr==NULL){
		return false;
	}


	*(--new_stackptr) = 1<<24;
	*(--new_stackptr) = (uint32_t)function_ptr;

	for (int i = 0; i < 14; i++) {
	    if (i == 5) {
//	    	uint32_t* args = 0xBA5EBA11;
	        *(--new_stackptr) = args;
	        continue;
	    }
	    *(--new_stackptr) = 0xA;
	}
	globalThread[stack_count].sp = new_stackptr;
	globalThread[stack_count].thread_function = function_ptr;
	globalThread[stack_count].timeslice = TIMESLICE;
	globalThread[stack_count].runtime = TIMESLICE;

	stack_count++;
	return true;
}


bool osCreateThreadWithDeadline(void (*function_ptr)(void*), void* args, uint32_t time){

	uint32_t* new_stackptr = New_Stack_Allocation(MSP_INIT_VAL);
	if(new_stackptr==NULL){
		return false;
	}


	*(--new_stackptr) = 1<<24;
	*(--new_stackptr) = (uint32_t)function_ptr;

	for (int i = 0; i < 14; i++) {
	    if (i == 5) {
//	    	uint32_t* args = 0xBA5EBA11;
	        *(--new_stackptr) = args;
	        continue;
	    }
	    *(--new_stackptr) = 0xA;
	}
	globalThread[stack_count].sp = new_stackptr;
	globalThread[stack_count].thread_function = function_ptr;
	globalThread[stack_count].timeslice = time;
	globalThread[stack_count].runtime = time;

	stack_count++;
	return true;
}


void osKernelInitialize(){
	MSP_INIT_VAL = *(uint32_t**)0x0;
	curr_sp = MSP_INIT_VAL;
	stack_pool = 0;

	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}

void osKernelStart() {
    // Check if the function pointer is not NULL before calling
    if (globalThread->thread_function != NULL) {
        // Call the function with a placeholder argument (can be NULL)
//        globalThread.thread_function(NULL);
    	print_cont();
    }
}

void osSched(){
	globalThread[stack_index].sp = (uint32_t*)(__get_PSP() - 8*4);
	stack_index = (stack_index+1)%stack_count;
	__set_PSP(globalThread[stack_index].sp);
}


uint32_t* New_Stack_Allocation(uint32_t* MSP_INIT_VAL) {

    if (curr_sp > (MSP_INIT_VAL - TOTAL_STACK)) {
    	curr_sp -= 0x400;
    	stack_pool += 0x400;
        return curr_sp;
    } else {
        return NULL;
    }
}

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2,(uint8_t*)&ch,1,HAL_MAX_DELAY);
	return ch;
}

void print_continuosly(){
	while(1){
		printf("Hello, PC\r\n");
	}
}

void print_t1(void* args){

    struct baseball input1 = *(struct baseball*)args;

    while(1)
    {
        printf("%hx\r\n", input1.base);
        osYield();
    }
}

void print_t2(){

	while(1)
	{
		printf("Input2\r\n");
	}
}


void print_failure(void)
{
	__asm("SVC #0");
}


void print_no_love(void)
{
	__asm("SVC #1");
}

void print_cont(void)
{
	__asm("SVC #3");
}

void osYield(void){
	__asm("SVC #4");
	//_ICSR |= 1<<28;
	//__asm("isb");

}

void jumpAssembly(void* fcn)
{
	__asm("MOV PC, R0");
}


void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case 17: //17 is sort of arbitrarily chosen
			printf("Success!\r\n");
			break;
		case 0:
			printf("You are a failure!\r\n");
			break;
		case 1:
			printf("Nobody loves you.\r\n");
			break;
		case 3:
			__set_PSP((uint32_t)globalThread->sp);
			runFirstThread();
			break;

		case 4:
			//Pend an interrupt to do the context switch
			_ICSR |= 1<<28;
			__asm("isb");
			break;
		default: /* unknown SVC */
			break;

	}
}


