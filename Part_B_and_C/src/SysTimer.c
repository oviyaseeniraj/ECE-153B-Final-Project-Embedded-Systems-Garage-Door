/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#include "stm32l476xx.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "LED.h"
#include "DMA.h"
#include "UART.h"
#include "motor.h"
#include "SPI.h"
#include "I2C.h"
#include "accelerometer.h"
#include <stdio.h>
#include <string.h>

#define TEMP_HIGH 35
#define TEMP_LOW 15
#define OPEN_THRES -1.04
#define CLOSE_THRES -1.8
#define CLOSE 0
#define OPEN 1
#define PAUSE -1
static char buffer[IO_SIZE];
static uint32_t volatile step;

void SysTick_Init(void) {
	// SysTick Control & Status Register
	SysTick->CTRL = 0; // Disable SysTick IRQ and SysTick Counter

	SysTick->LOAD = 79999;
	SysTick->VAL = 0;
	
	// Enables SysTick exception request
	// 1 = counting down to zero asserts the SysTick exception request
	// 0 = counting down to zero does not assert the SysTick exception request
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 
	// Select clock source
	// If CLKSOURCE = 0, the external clock is used. The frequency of SysTick clock is the frequency of the AHB clock divided by 8.
	// If CLKSOURCE = 1, the processor clock is used.
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;		
	
	// Enable SysTick IRQ and SysTick Timer
	// Configure and Enable SysTick interrupt in NVIC
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_SetPriority(SysTick_IRQn, 0); // Set Priority to 1
}

void SysTick_Handler(void) {
	//TODO
	++step;
  rotate(); // motor updates periodically
}

void delay(uint32_t ms) {
	uint32_t currentTicks = step;
	
	while ((step - currentTicks) < ms) {
		// wait
	}
	return;
}

void delay3sOpen()
{
	double x, y, z;
	readValues(&x, &y, &z);
	if (z < OPEN_THRES)
	{
		setDire(OPEN);
	}
	else
	{
		setDire(PAUSE);
	}
	delay(3000);
	sprintf(buffer, "3 seconds have passed. Device will resume normal behavior.");
	UART_print(buffer);
}

void delay3sClose()
{
	double x, y, z;
	readValues(&x, &y, &z);
	if (z > CLOSE_THRES)
	{
		setDire(CLOSE);
	}
	else
	{
		setDire(PAUSE);
	}
	delay(3000);
	sprintf(buffer, "3 seconds have passed. Device will resume normal behavior.");
	UART_print(buffer);
}