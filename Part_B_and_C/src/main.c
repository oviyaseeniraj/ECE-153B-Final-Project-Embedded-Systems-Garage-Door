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
 
static char buffer[IO_SIZE];

#define TEMP_HIGH 35
#define TEMP_LOW 15
#define OPEN_THRES -1.04
#define CLOSE_THRES -1.7
#define CLOSE 0
#define OPEN 1
#define PAUSE -1

int temp_th = 27;
int duringDelay = 0;

void getTemperatureData(int* temp);
void openDoor(void);
void closeDoor(void);

void getTemperatureData(int* temp) {
	// "<< 1" needed bc bit 0 is treated as a don't care in 7-bit addressing mode
	uint8_t SecondaryAddress = 0b1001000 << 1;
	uint8_t Data_Send = 0x00;
	uint8_t Data_Receive;
	
	// send read command to sensor
	Data_Send = 0x00;
	I2C_SendData(I2C1, SecondaryAddress, &Data_Send, 1); 
	
	// get measurement
	I2C_ReceiveData(I2C1, SecondaryAddress, &Data_Receive, 1);
	
	*temp = 0x7F & Data_Receive;
}

void openDoor()
{
	sprintf(buffer, "Door opening.\n");
	UART_print(buffer);
	double x, y, z;
	readValues(&x, &y, &z);
	if (z < OPEN_THRES)
	{
		setDire(OPEN);
	}
	else
	{
		setDire(PAUSE);
		sprintf(buffer, "Door opened.");
		UART_print(buffer);
	}
	temp_th = TEMP_LOW;
}

void closeDoor()
{
	sprintf(buffer, "Door closing.\n");
	UART_print(buffer);
	double x, y, z;
	readValues(&x, &y, &z);
	if (z > CLOSE_THRES)
	{
		setDire(CLOSE);
	}
	else
	{
		setDire(PAUSE);
		sprintf(buffer, "Door closed.\n");
		UART_print(buffer);
	}
	temp_th = TEMP_HIGH;
}

void UART_onInput(char* inputs, uint32_t size)
{
	UART_print(buffer);
	double x, y, z;
	readValues(&x, &y, &z);
	//check input string, print out console message, rotate motor, print out console message, wait 3 seconds
	if(strcmp(inputs, "open\r\n") == 0)
	{
		duringDelay = 1;
		closeDoor();
		delay3sOpen();
		temp_th = TEMP_LOW;
		UART1_Init();
	}
	else if(strcmp(inputs, "close\r\n") == 0)
	{
		duringDelay = 1;
		closeDoor();
		delay3sClose();
		temp_th = TEMP_HIGH;
		UART1_Init();
	}
	else
	{
		setDire(PAUSE);
		sprintf(buffer, "Not valid input (open, close). Stopping door.\n");
		UART_print(buffer);
	}
	duringDelay = 0;
}

int main(void) {
	double x,y,z;
	
	//Temperature Sensor Variables
	int prevTemp = 0;
	int temp = 0;
	
	// Switch System Clock = 80 MHz
	System_Clock_Init(); 
	SysTick_Init();
	UART1_Init();
	LED_Init();	
	Motor_Init();
	SPI1_GPIO_Init();
	SPI1_Init();
	initAcc();
	I2C_GPIO_Init();
	I2C_Initialization();
	
	//start off in close
	setDire(0);
	temp_th = TEMP_HIGH;
	sprintf(buffer, "Program Starts.\n");
	UART_print(buffer);
	
	while(1)
	{
		//accelerometer: stop door when in correct position
		readValues(&x, &y, &z);
		
		sprintf(buffer, "X: %.2f, Y: %.2f, Z: %.2f\n", x, y, z);
		UART_print(buffer);
	
		
		if (z > OPEN_THRES && getDire() == 1)
		{
			setDire(-1);
			sprintf(buffer, "Door opened.");
			UART_print(buffer);
		}
		if (z < CLOSE_THRES && getDire() == 0)
		{
			setDire(-1);
			sprintf(buffer, "Door closed.");
			UART_print(buffer);
		}
		
		
		//print temperature when it changes
		getTemperatureData(&temp);
		if (temp != prevTemp)
		{
			sprintf(buffer, "Temperature changed: %d\n", temp);
			UART_print(buffer);
			
			if (duringDelay == 0 && temp > TEMP_HIGH && temp_th == TEMP_HIGH && getDire() != OPEN)
			{
				sprintf(buffer, "Temperature too high. ");
				UART_print(buffer);
				openDoor();
			}
			else if (duringDelay == 0 && temp < TEMP_LOW && temp_th == TEMP_LOW && getDire() != CLOSE)
			{
				sprintf(buffer, "Temperature too low. ");
				UART_print(buffer);
				closeDoor();
			}
		}
		prevTemp = temp;
		
			
		delay(1000);
	}
}


