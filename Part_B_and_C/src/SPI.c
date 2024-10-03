#include "SPI.h"
#include "SysTimer.h"

void SPI1_GPIO_Init(void) {
	// Enable the GPIO Clock
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	// Set PA4, PB3, PB4, and PB5 to Alternative Functions, and configure their AFR to SPI1
  //PA4
  GPIOA->MODER &= ~(GPIO_MODER_MODE4); // Clear MODER5[1:0] for PB5
  GPIOA->MODER |= (GPIO_MODER_MODE4_1); //Set to alternative function
  //AF5 = 0101, write this to afsel ports
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_0;
  //GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL4_1;
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_2;
  //GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL4_3;
  
  //PB3
  GPIOB->MODER &= ~(GPIO_MODER_MODE3); // Clear MODER5[1:0] for PB3
  GPIOB->MODER |= (GPIO_MODER_MODE3_1); //Set to alternative function
  //AF5 = 0101, write this to afsel ports
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_0;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL3_1;
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_2;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL3_3;
  
  //PB4
  GPIOB->MODER &= ~(GPIO_MODER_MODE4); // Clear MODER5[1:0] for PB4
  GPIOB->MODER |= (GPIO_MODER_MODE4_1); //Set to alternative function
  //AF5 = 0101, write this to afsel ports
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL4_0;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL4_1;
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL4_2;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL4_3;
  
  //PB5
  GPIOB->MODER &= ~(GPIO_MODER_MODE5); // Clear MODER5[1:0] for PB4
  GPIOB->MODER |= (GPIO_MODER_MODE5_1); //Set to alternative function
  //AF5 = 0101, write this to afsel ports
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL5_0;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL5_1;
  GPIOB->AFR[0] |= GPIO_AFRL_AFSEL5_2;
  //GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL5_3;
  
  
	// Set GPIO Pins to: Very High Output speed, Output Type Push-Pull, and No Pull-Up/Down
  
  //pa4
  GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
  GPIOA->OTYPER &= ~(GPIO_OTYPER_OT4);
  GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD4);
  
  //pb3
  GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT3);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3);
  
  //pb4
  GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT4);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD4);
  
  //pb5
  GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED5;
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT5); // pull up pull down output type
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD5); // no pull up no pull down
}


void SPI1_Init(void){
	// Enable SPI clock and Reset SPI
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
  
	// Disable SPI
	SPI1->CR1 &= ~SPI_CR1_SPE;
	
	// Configure for Full Duplex Communication
	SPI1->CR1 &= ~SPI_CR1_RXONLY;
	// Configure for 2-line Unidirectional Data Mode
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;
	// Disable Output in Bidirectional Mode
	SPI1->CR1 &= ~SPI_CR1_BIDIOE;
	// Set Frame Format: MSB First, 16-bit, Motorola Mode
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;
	SPI1->CR2 |= SPI_CR2_DS;
	SPI1->CR2 &= ~SPI_CR2_FRF;
	// Configure Clock. Read DataSheet for required value
	SPI1->CR1 |= SPI_CR1_CPOL;
	SPI1->CR1 |= SPI_CR1_CPHA;
	
	// Set Baud Rate Prescaler to 16
	SPI1->CR1 &= ~SPI_CR1_BR;
	SPI1->CR1 |= SPI_CR1_BR_0;
	SPI1->CR1 |= SPI_CR1_BR_1;
	// Disable Hardware CRC Calculation
	SPI1->CR1 &= ~SPI_CR1_CRCEN;
	// Set as Master
	SPI1->CR1 |= SPI_CR1_MSTR;
	// Disable Software Slave Management
	SPI1->CR1 &= ~SPI_CR1_SSM;
	// Enable NSS Pulse Management
	SPI1->CR2 |= SPI_CR2_NSSP;
	// Enable Output
	SPI1->CR2 |= SPI_CR2_SSOE;
	// Set FIFO Reception Threshold to 1/2
	SPI1->CR2 &= ~SPI_CR2_FRXTH;
	// Enable SPI
	SPI1->CR1 |= SPI_CR1_SPE;
}

uint16_t SPI_Transfer_Data(uint16_t write_data){
	// Wait for TXE (Transmit buffer empty)
	while ((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE)
	{
		//wait
	}
	// Write data
	SPI1->DR ^= (SPI1->DR ^ write_data);
	// Wait for not busy
	while ((SPI1->SR & SPI_SR_BSY) == SPI_SR_BSY)
	{
		//wait
	}
	// Wait for RXNE (Receive buffer not empty)
	while ((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE)
	{
		//wait
	}
	// Read data
	uint16_t read_data = 0xFFFF & SPI1->DR;
	return read_data;
}