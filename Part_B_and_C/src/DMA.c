/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */
 
#include "DMA.h"
#include "UART.h"
#include "SysTimer.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  
  // Wait 20us
  for (volatile uint32_t i = 0; i < 1000; ++i);
  
	//channel 6
  //disable
  tx->CCR &= ~DMA_CCR_EN;
  
  //disable memory-to-memory mode
  tx->CCR &= ~DMA_CCR_MEM2MEM;
  
  //high priority
  tx->CCR &= ~DMA_CCR_PL;
  
  // peripheral size =8-bit
  tx->CCR &= ~DMA_CCR_PSIZE;
  
  //memory size = 8-bit
  tx->CCR &= ~DMA_CCR_MSIZE;
  
  //disable peripheral increment mode
  tx->CCR &= ~DMA_CCR_PINC;
  
  //enable memory increment mode
  tx->CCR |= DMA_CCR_MINC;
  
  //disable circular mode
  tx->CCR &= ~DMA_CCR_CIRC;
  
  //Memory-to-Peripheral
  tx->CCR |= DMA_CCR_DIR;
  
  //data source DO I NEED THIS
  //DMA1_Channel6->CMAR = (uint32_t)DataBuffer;
  
  //data destination
  tx->CPAR = (uint32_t)&(uart->TDR);
  
	//dma interrupt
  //disable half transfer interrupt
  tx->CCR &= ~DMA_CCR_HTIE;
  
  //disable transfer error interrupt
  tx->CCR &= ~DMA_CCR_TEIE;
  
  //enable transfer complete interrupt
  tx->CCR &= ~DMA_CCR_TCIE;
  
	//NVIC_ClearPendingIRQ(DMA1_Channel7_IQRn);
  NVIC_SetPriority(DMA1_Channel7_IRQn, 2);
	NVIC_SetPriority(DMA1_Channel4_IRQn, 2);
	
	//NVIC_ClearPendingIRQ(DMA1_Channel4_IQRn);
  
  //enable interrupt in NVIC
  NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

void DMA1_Channel7_IRQHandler(void){ 
	// TODO
	//clear NVIC interrupt flag
  NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);
  
  //Transfer Complete interrupt flag
  if ((DMA1->ISR & DMA_ISR_TCIF7) != 0) {
    //clear flag
    DMA1->IFCR = DMA_IFCR_CTCIF7;
    // mark computation completed
		on_complete_transfer();
    //completeCRC(CRC->DR);
  }
  //clear global DMA interrupt flag
  DMA1->IFCR = DMA_IFCR_CGIF7;
}

void DMA1_Channel4_IRQHandler(void)
{
	//clear NVIC interrupt flag
  NVIC_ClearPendingIRQ(DMA1_Channel4_IRQn);
  
  //Transfer Complete interrupt flag
  if ((DMA1->ISR & DMA_ISR_TCIF4) != 0) {
    //clear flag
    DMA1->IFCR = DMA_IFCR_CTCIF4;
    // mark computation completed
		on_complete_transfer();
    //completeCRC(CRC->DR);
  }
  //clear global DMA interrupt flag
  DMA1->IFCR = DMA_IFCR_CGIF4;
}