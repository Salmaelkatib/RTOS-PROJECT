#include "Init.h"

void Delay_ms(int time_ms)
{
		__disable_irq();
    int i, j;
    for(i = 0 ; i < time_ms; i++)
        for(j = 0; j < 3180; j++)
            {}  /* excute NOP for 1ms */
		__enable_irq();
}

void portF_init(void){    
  SYSCTL_RCGCGPIO_R |= 0x20;
  while ((SYSCTL_PRGPIO_R & 0x20) != 0x20);
  GPIO_PORTF_LOCK_R |= 0X4C4F434B;
  GPIO_PORTF_CR_R |= (1 <<0 | 1<<1| 1<<2 | 1<<3 );
  GPIO_PORTF_DIR_R |= 0x00;   //0,1,2 input
  GPIO_PORTF_PDR_R |=  (1 <<0 | 1<<1| 1<<2 );
  GPIO_PORTF_DEN_R |= (1 <<0 | 1<<1| 1<<2| 1<<3);
	
	//enable interrupt 3la pin 1,2 for lock and jamming buttons
	//GPIOF->IS  |= (1<<1);                       /* make bits level sensitive for LOCK_BTN*/
	GPIOF->IS  &= ~(1<<2) | ~(1<<1);                       /* make bits edge sensitive for JAMMING_BTN*/
	GPIOF->IBE &= ~(1<<1) |~(1<<2);                     /* trigger is controlled by IEV */
  GPIOF->IEV |= (1<<2) | (1<<1) ;                      /* rising edge trigger as we configure pin in pull down mode */
	GPIOF->ICR |= (1<<1) |(1<<2);                     /* clear any prior interrupt */
  GPIOF->IM  |= (1<<1) |(1<<2);                 /* unmask interrupt */
	NVIC_EN0_R |= 1<<30;        /*Enable PORTF Interrupt IRQ30 */
	NVIC_PRI7_R =0xE00000;	
}
void portA_init(void){  
  SYSCTL_RCGCGPIO_R |= 0x01;
  while ((SYSCTL_PRGPIO_R & 0x01) != 0x01);
  GPIO_PORTA_LOCK_R |= 0X4C4F434B;
  GPIO_PORTA_CR_R |= (1 <<2 |1 <<3 | 1<<5 | 1<<6);
  GPIO_PORTA_DIR_R |= 0x0C;            //2 and 3 are output & 5 and 6 are input
  GPIO_PORTA_PDR_R |= ( 1<<5 | 1<<6 );
  GPIO_PORTA_DEN_R |= (1 <<2 |1 <<3 | 1<<5 | 1<<6 );
	
	//enable interrupt 3la pin 5,6 for passenger up/down buttons
	GPIOA->IS  &= ~(1<<5)|~(1<<6);      /* make bits edge sensitive */
	GPIOA->IBE &= ~(1<<5)|~(1<<6);      /* trigger is controlled by IEV */
  GPIOA->IEV |= (1<<5)|(1<<6);       /* rising edge trigger as we configure pin in pull down mode */
	GPIOA->ICR |= (1<<5)|(1<<6);  
  GPIOA->IM = 0x60;                 /* unmask interrupt */
	NVIC_EN0_R = 0x01;               /*Enable PORTA Interrupt IRQ0 */
	NVIC_PRI7_R =0xE00000;
}
void portE_init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x10;
  while ((SYSCTL_PRGPIO_R & 0x10) != 0x10);
  GPIO_PORTE_LOCK_R |= 0X4C4F434B;
  GPIO_PORTE_CR_R |= 0x0E;
  GPIO_PORTE_DIR_R |= 0x08; 
  GPIO_PORTE_PDR_R |= 0x06;
  GPIO_PORTE_DEN_R = 0x0E;
	
	//enable interrupt 3la pin 2,1 for driver up/down buttons
	GPIOE->IS  &= ~(1<<2) |~(1<<1);      /* make bits edge sensitive */
	GPIOE->IBE &= ~(1<<2) |~(1<<1);      /* trigger is controlled by IEV */
  GPIOE->IEV |=  (1<<2) |(1<<1);      /* rising edge trigger as we configure pin in pull down mode */
	GPIOE->ICR |=  (1<<2) | (1<<1);      /* clear any prior interrupt */
  GPIOE->IM  |=  (1<<2) | (1<<1);          /* unmask interrupt */
	NVIC_EN0_R |= 1<<4;                       /*Enable PORTE Interrupt IRQ4 */
	NVIC_PRI7_R =0xE00000;
}
