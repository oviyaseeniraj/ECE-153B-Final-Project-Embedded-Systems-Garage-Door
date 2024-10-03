/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */


#include "UART.h"
#include "DMA.h"
#include <string.h>
#include <stdio.h>

static volatile DMA_Channel_TypeDef * tx;
static volatile char inputs[IO_SIZE];
static volatile uint8_t data_t_0[IO_SIZE];
static volatile uint8_t data_t_1[IO_SIZE];
static volatile uint8_t input_size = 0;
static volatile uint8_t pending_size = 0;
static volatile uint8_t * active = data_t_0;
static volatile uint8_t * pending = data_t_1;

static char buffer[IO_SIZE];

#define SEL_0 1
#define BUF_0_EMPTY 2
#define BUF_1_EMPTY 4
#define BUF_0_PENDING 8
#define BUF_1_PENDING 16

void transfer_data(char ch);
void on_complete_transfer(void);

void UART1_Init(void)
{
	UART1_GPIO_Init();
	
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	RCC->CCIPR &= ~(RCC_CCIPR_USART1SEL);
	RCC->CCIPR |= RCC_CCIPR_USART1SEL_0;
	
	tx = DMA1_Channel4;
	
	DMA_Init_UARTx(tx, USART1);
	tx->CMAR = (uint32_t)active;
	DMA1_CSELR->CSELR &= ~DMA_CSELR_C7S;
	DMA1_CSELR->CSELR |= (1<<13);
	
	USART_Init(USART1);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 1);
}

void UART2_Init(void)
{
	UART2_GPIO_Init();
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

	RCC->CCIPR &= ~(RCC_CCIPR_USART2SEL);
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
	
	tx = DMA1_Channel7;
	
	DMA_Init_UARTx(tx, USART2);
	tx->CMAR = (uint32_t)active;
	DMA1_CSELR->CSELR &= ~DMA_CSELR_C7S;
	DMA1_CSELR->CSELR |= (1<<25);
	
	USART_Init(USART2);
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 1);
}

void UART1_GPIO_Init(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;
	
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6_0 | GPIO_AFRL_AFSEL6_1 | GPIO_AFRL_AFSEL6_2;
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL7_0 | GPIO_AFRL_AFSEL7_1 | GPIO_AFRL_AFSEL7_2;

	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
	
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0;
}

void UART2_GPIO_Init(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOA->MODER |= GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1;
	
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2;

	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
	
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0 | GPIO_PUPDR_PUPD3_0;
}

void USART_Init(USART_TypeDef * USARTx)
{ 
	USARTx->CR1 &= ~(USART_CR1_UE);

	USARTx->CR1 &= ~USART_CR1_M;
	
	USARTx->CR3 |= USART_CR3_DMAT;
	
	USARTx->CR1 &= ~(USART_CR1_OVER8);
	USARTx->CR2 &= ~(USART_CR2_STOP);
	
	USARTx->BRR = 8333;

	USARTx->CR1 |= USART_CR1_TE;
	USARTx->CR1 |= USART_CR1_RE;
	USARTx->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE;

	USARTx->CR1 |= USART_CR1_UE;
}

/**
 * This function accepts a string that should be sent through UART
*/
void UART_print(char* data)
{
		uint8_t data_length = 0;

    while (data[data_length] != '\0')
		{
			data_length++;
    }
		
    if (!(tx->CCR & DMA_CCR_EN))
		{ // Check DMA status. If DMA is ready, send data
			for (uint8_t i = 0; i < data_length; i++)
			{ // chars to buffer
				active[i] = (uint8_t)data[i];
			}	
			
			tx->CMAR = (uint32_t)active;
			tx->CNDTR = data_length;
			tx->CCR |= DMA_CCR_EN;
			for (int i = 0; i < 100000; i++) {}
    }
    else
		{ //If DMA is not ready, put the data aside		
			for (uint8_t i = pending_size; i < data_length + pending_size; i++)
			{
				if(i - pending_size >= 0 && i < IO_SIZE)
				{
					pending[i] = (uint8_t)data[i - pending_size];
				}
      }
        pending_size += data_length;
    }
}

/**
 * This function should be invoked when a character is accepted through UART
*/
void transfer_data(char ch) {
	inputs[input_size] = ch; // Append character to input buffer.
	input_size++;                                                                                                                                                                     
	
	if (input_size > 0 && ch == '\n')
	{ // If the character is end-of-line, invoke UART_onInput
		UART_onInput(inputs, input_size);
		input_size = 0;
		memset(inputs,0,strlen(inputs));
	}
}

/**
 * This function should be invoked when DMA transaction is completed
*/
void on_complete_transfer(void)
{
	tx->CCR &= ~DMA_CCR_EN;
	if (pending_size > 0)
	{ // If there are pending data to send, switch active and pending buffer, and send data
		uint8_t * temp = active;
		active = pending;
		pending = temp;
		memset(pending,0,strlen(pending));
		pending_size = 0;
		UART_print(active);
	}
}

void USART1_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(USART1_IRQn);
	
	if (USART1->ISR & USART_ISR_RXNE)
	{ // When receive a character, invoke transfer_data
		char ch = USART1->RDR;
		transfer_data(ch);
	}
	
	if (USART1->ISR & USART_ISR_TC)
	{ // When complete sending data, invoke on_complete_transfer
		USART1->ICR |= USART_ICR_TCCF;
		on_complete_transfer();
	}
}

void USART2_IRQHandler(void)
{	
	NVIC_ClearPendingIRQ(USART2_IRQn);
	
	if (USART2->ISR & USART_ISR_RXNE)
	{ // When receive a character, invoke transfer_data
		char ch = USART2->RDR;
		transfer_data(ch);
	}
	
	if (USART2->ISR & USART_ISR_TC)
	{ // When complete sending data, invoke on_complete_transfer
		USART2->ICR |= USART_ICR_TCCF;
		on_complete_transfer();
  }
}

void UART_transferReceivedData(void)
{
	if ((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{ // if receive buffer not empty. 
		transfer_data(USART1->RDR);
	}
}