/*
 * kernel.h
 *
 *  Created on: Oct 17, 2023
 *      Author: domen
 */

#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV
#define TIMESLICE 5
#include <stdint.h>
#include <stdbool.h>

typedef struct k_thread{
	uint32_t* sp; //stack pointer
	void (*thread_function)(void*);
	uint32_t timeslice;
	uint32_t runtime;
}thread;



void SVC_Handler_Main(unsigned int *svc_args);

bool osCreateThread(void (*function_ptr)(void*), void* args);

int __io_putchar(int ch);

void print_continuosly();

void print_failure(void);

void print_no_love(void);

void print_cont(void);

void print_t1();

void print_t2();

void osYield(void);

void jumpAssembly(void* fcn);

uint32_t* New_Stack_Allocation(uint32_t* MSP_INIT_VAL);


#endif /* INC_KERNEL_H_ */
