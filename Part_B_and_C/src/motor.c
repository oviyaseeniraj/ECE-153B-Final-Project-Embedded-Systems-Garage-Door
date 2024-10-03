/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#include "stm32l476xx.h"
#include "motor.h"

static const uint32_t MASK = GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD8 | GPIO_ODR_OD9;
static const uint32_t HalfStep[8] = {0b1000, 0b1010, 0b0010, 0b0110, 0b0100, 0b0101, 0b0001, 0b1001};

static volatile int8_t dire = 0;
static volatile uint8_t step = 0;

void Motor_Init(void) {	
	//TODO
	// Enable GPIO Clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // Configure pins 5, 6, 8, and 9 as output
    GPIOC->MODER &= ~GPIO_MODER_MODE5;
    GPIOC->MODER |= GPIO_MODER_MODE5_0;
    GPIOC->MODER &= ~GPIO_MODER_MODE6;
    GPIOC->MODER |= GPIO_MODER_MODE6_0;
    GPIOC->MODER &= ~GPIO_MODER_MODE8;
    GPIOC->MODER |= GPIO_MODER_MODE8_0;
    GPIOC->MODER &= ~GPIO_MODER_MODE9;
    GPIOC->MODER |= GPIO_MODER_MODE9_0;

    // Set output speeds to fast
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5;
    GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_1;
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED6;
    GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_1;
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8;
    GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED8_1;
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED9;
    GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED9_1;

    // Set output type to push-pull
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT5;
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT6;
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT8;
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT9;

    // Set to no pull-up, no pull-down
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD5;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD6;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD8;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD9;

    GPIOC->ODR &= ~MASK;
}

void rotate(void) {
	//TODO
	// Activate the pins based on the current step
    uint32_t outputData = 0;

    if (0b0001 & HalfStep[step])
        outputData |= GPIO_ODR_ODR_5;

    if (0b0010 & HalfStep[step])
        outputData |= GPIO_ODR_ODR_6;

    if (0b0100 & HalfStep[step])
        outputData |= GPIO_ODR_ODR_8;

    if (0b1000 & HalfStep[step])
        outputData |= GPIO_ODR_ODR_9;

    GPIOC->ODR &= ~(GPIO_ODR_ODR_5 | GPIO_ODR_ODR_6 | GPIO_ODR_ODR_8 | GPIO_ODR_ODR_9); // Turn off all pins first
    GPIOC->ODR |= outputData; // Activate the required pins based on the step

    // Increment or decrement the current step based on the direction
    if (dire == 0) {
        step++;
        if (step >= 8)
            step = 0;
    } else if (dire == -1) {
			step = step;
		}
		else {
        if (step == 0)
            step = 7;
        else
            step--;
    }
}

void setDire(int8_t direction) {
	//TODO
	dire = direction;
}

int8_t getDire(void)
{
	return dire;
}

void configureTimer(void) {
    // Enable the clock for TIM3
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

    // Set the timer prescaler and period
    TIM3->PSC = 200; // Adjust the prescaler based on the desired frequency
    TIM3->ARR = 1000; // Adjust the period based on the desired speed

    // Enable the update interrupt
    TIM3->DIER |= TIM_DIER_UIE;

    // Set the interrupt priority and enable the interrupt
    NVIC_SetPriority(TIM3_IRQn, 0);
    NVIC_EnableIRQ(TIM3_IRQn);

    // Start the timer
    TIM3->CR1 |= TIM_CR1_CEN;
}