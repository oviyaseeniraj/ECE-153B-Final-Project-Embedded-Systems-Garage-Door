/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#include "LED.h"

void LED_Init(void) {
	// [TODO]
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // Enable Clock for GPIO Port A (LED)
	
	// Initialize Green LED
	GPIOA->MODER &= ~(GPIO_MODER_MODE5); // Clear MODER5[1:0] for PA5
  GPIOA->MODER |= (GPIO_MODER_MODE5_0);  // Set PA5 as General purpose output mode
  GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5);      // Set PA5 to push-pull
  GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5); // No pull-up, no pull-down
}

void LED_Off(void) {
	// [TODO]
	GPIOA->ODR &= ~GPIO_ODR_OD5; // Turn off the LED
}

void LED_On(void) {
	// [TODO]
	GPIOA->ODR |= GPIO_ODR_OD5; // Turn on the LED
}

void LED_Toggle(void) {
	// [TODO]
	GPIOA->ODR ^= GPIO_ODR_OD5; // Toggle = XOR w itself
}

