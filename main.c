#include <stdint.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "driver.h"
#define Get_Bit(Register, Bit) (( Register & (1 << Bit)) >> Bit)
#define mainSW_INTURRUPT_PortF ((IRQn_Type)30)
#define mainSW_INTURRUPT_PortE ((IRQn_Type)4)
#define mainSW_INTURRUPT_PortA ((IRQn_Type)0)

unsigned char ReadPin(volatile unsigned long*  Port, int Pin){
  return Get_Bit(*Port, Pin);
}

xSemaphoreHandle xMutex;
xSemaphoreHandle xBinarySemaphore1;  //Jamming Semaphore
xSemaphoreHandle xBinarySemaphore2;  //Passenger up semaphore
xSemaphoreHandle xBinarySemaphore3;  //Passenger down semaphore
xSemaphoreHandle xBinarySemaphore4;  //Driver up semaphore
xSemaphoreHandle xBinarySemaphore5;  //Driver down semaphore
xSemaphoreHandle xBinarySemaphore6;  //Lock semaphore
int Locked=0;

void PassengerUp (void *pvParameters);
void PassengerDown (void *pvParameters);
void DriverUp (void *pvParameters);
void DriverDown (void *pvParameters);
void Jamming (void *pvParameters);
void Motor_Control( int num );
void vApplicationIdleHook(void);

void Motor_Control( int num ){
	
	switch(num){
		case 0:  //stop
		{ GPIOE->DATA =0x00;
			GPIOA->DATA =0x0C;
			break;
		}
		case 1:  //clockwise - Window up
		{ GPIOE->DATA =0x08;
			GPIOA->DATA =0x08;
			break;
		}
		case 2:  //anti-clockwise - Window down
		{
			GPIOE->DATA =0x08;
		  GPIOA->DATA =0x04;
			break;
		}
	}
}

void PassengerUp (void *pvParameters)
{
	xSemaphoreTake( xBinarySemaphore2, 0 ); 
	for( ;; )  {
			xSemaphoreTake( xBinarySemaphore2, portMAX_DELAY );  
			xSemaphoreTake(xMutex , portMAX_DELAY);	
		 
			Motor_Control(1);
		NVIC_EN0_R &=~ (1<<0);        /*Disable PORTA Interrupt IRQ0 */
		//while LIMIT_SWITCH is not pressed and PASSENGER_UP is still pressed
		 while( ReadPin(&GPIO_PORTF_DATA_R,0)==0x00  && ReadPin(&GPIO_PORTA_DATA_R,5) ==0x01){
			 }
		 NVIC_EN0_R |= 1<<0;        /*Enable PORTA Interrupt IRQ0 */
		  //stop
			 Motor_Control(0);
			xSemaphoreGive(xMutex);
	}
}

void PassengerDown (void *pvParameters)
{
	xSemaphoreTake( xBinarySemaphore3, 0 ); 
	for( ;; )  {	 
		xSemaphoreTake( xBinarySemaphore3, portMAX_DELAY );    
		xSemaphoreTake(xMutex , portMAX_DELAY);
		Motor_Control(2);
		
		NVIC_EN0_R &=~ (1<<0);        /*Disable PORTA Interrupt IRQ0 */
		//while LIMIT_SWITCH is not pressed and PASSENGER_DOWN is still pressed
		 while( ReadPin(&GPIO_PORTF_DATA_R,0)==0x00  && ReadPin(&GPIO_PORTA_DATA_R,6) ==0x01){
		 }
		NVIC_EN0_R |= (1<<0);        /*Enable PORTA Interrupt IRQ0 */
		 
			//Stop()
			Motor_Control(0);
		 xSemaphoreGive(xMutex);
		}
	}

void DriverUp (void *pvParameters){

 	xSemaphoreTake( xBinarySemaphore4, 0 );
	for(;;){		
		 xSemaphoreTake( xBinarySemaphore4, portMAX_DELAY );
			xSemaphoreTake(xMutex , portMAX_DELAY);
 			 Motor_Control(1);
		 NVIC_EN0_R &=~ (1<<4);        /*Disable PORTE Interrupt IRQ4 */
			 Delay_ms(500);
		
			// In manual mode
		if(ReadPin(&GPIO_PORTE_DATA_R,2)==0x01){
			// while LIMIT_SWITCH not pressed and DRIVER_UP is still pressed   
			 while((ReadPin(&GPIO_PORTF_DATA_R,0)==0x00 && ReadPin(&GPIO_PORTE_DATA_R,2)==0x01)){
		 }
			NVIC_EN0_R |= (1<<4);        /*Enable PORTE Interrupt IRQ4 */
			//Stop
			Motor_Control(0);
		}
		else if(ReadPin(&GPIO_PORTE_DATA_R,2)==0x00){
			//In auto mode
			// while LIMIT_SWITCH not pressed
			while(ReadPin(&GPIO_PORTF_DATA_R,0)==0x00){
			}
			//Stop;
			Motor_Control(0);
		}
		xSemaphoreGive(xMutex);	
	}
}

void DriverDown (void *pvParameters){
	
		xSemaphoreTake( xBinarySemaphore5, 0 );
	for(;;){
	 	xSemaphoreTake( xBinarySemaphore5, portMAX_DELAY );	
	  xSemaphoreTake(xMutex , portMAX_DELAY);
		Motor_Control(2);
		 NVIC_EN0_R &=~ (1<<4);        /*Disable PORTE Interrupt IRQ4 */
    Delay_ms(500);
	 if(ReadPin(&GPIO_PORTE_DATA_R,1)==0x01){
		// In manual mode
		// while LIMIT_SWITCH not pressed and DRIVER_DOWN is pressed
		 while( ReadPin(&GPIO_PORTF_DATA_R,0)==0x00 && ReadPin(&GPIO_PORTE_DATA_R,1)==0x01){
   }
		 NVIC_EN0_R |= (1<<4);        /*Enable PORTE Interrupt IRQ4 */
		//Stop(); 
	 Motor_Control(0);
	}
	else if(ReadPin(&GPIO_PORTE_DATA_R,1)==0x00){
		//In auto mode
		// while LIMIT_SWITCH not pressed
		while(ReadPin(&GPIO_PORTF_DATA_R,0)==0x00){
		}
		//Stop
		Motor_Control(0);
	}
		xSemaphoreGive(xMutex);
}
}
void Jamming(void * pvParameters){
	portTickType xLastWakeTime; 
	xLastWakeTime = xTaskGetTickCount();
	xSemaphoreTake( xBinarySemaphore1, 0 ); 
	for(;;){
	xSemaphoreTake( xBinarySemaphore1, portMAX_DELAY );   
	// stop
  Motor_Control(0);	
	// window down
	Motor_Control(2);
	Delay_ms(2000);
	vTaskDelayUntil( &xLastWakeTime, ( 5000 / portTICK_RATE_MS ) );
	NVIC_EN0_R |= (1<<4);          /*Enable PORTE Interrupt IRQ4 */
  // stop
  Motor_Control(0);		
	}
}
void Lock (void *pvParameters)
{
	
	xSemaphoreTake(xBinarySemaphore6 ,0);
	for(;;){
		NVIC_EN0_R &=~ (1<<0);        /*Disable PORTA Interrupt IRQ0 */
		xSemaphoreTake(xBinarySemaphore6 ,portMAX_DELAY);
		if(ReadPin(&GPIO_PORTF_DATA_R,1 )==0x00)
			Locked =0;   //enable passenger panel
		else if(ReadPin(&GPIO_PORTF_DATA_R,1)==0x01)
			Locked =1;   //disable passenger panel
		NVIC_EN0_R |= (1<<0);        /*Enable PORTA Interrupt IRQ0 */
	} 
}

void GPIOE_Handler (void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
	 if (Get_Bit(GPIO_PORTE_MIS_R , 2))          //if DRIVER_UP is pressed
	{ 
		xSemaphoreGiveFromISR(xBinarySemaphore4,&xHigherPriorityTaskWoken);
		GPIO_PORTE_ICR_R |= 1<<2;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}
	else if (Get_Bit(GPIO_PORTE_MIS_R , 1))          //if DRIVER_DOWN is pressed
	{ 
		xSemaphoreGiveFromISR(xBinarySemaphore5,&xHigherPriorityTaskWoken);
		GPIO_PORTE_ICR_R |= 1<<1;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}
}	
void GPIOA_Handler(void){
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
	 if (Get_Bit(GPIO_PORTA_MIS_R , 5))          //if PASSENGER_UP is pressed
	{ 
		if(Locked==0){
		xSemaphoreGiveFromISR(xBinarySemaphore2,&xHigherPriorityTaskWoken);
		GPIO_PORTA_ICR_R |= 1<<5;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}		
}
	else if (Get_Bit(GPIO_PORTA_MIS_R , 6))          //if PASSENGER_DOWN is pressed
	{ 
		if(Locked==0){
		xSemaphoreGiveFromISR(xBinarySemaphore3,&xHigherPriorityTaskWoken);
		GPIO_PORTA_ICR_R |= 1<<6;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}
}
}	
void GPIOF_Handler()
{
	 portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
	 if (Get_Bit(GPIO_PORTF_MIS_R , 1) )         //if LOCK_BTN is pressed
	{ 
		xSemaphoreGiveFromISR(xBinarySemaphore6,&xHigherPriorityTaskWoken);
		GPIO_PORTF_ICR_R |= 1<<1;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}
	else if (Get_Bit(GPIO_PORTF_MIS_R ,2))              //if JAMMING_BTN is pressed
	{ 
		xSemaphoreGiveFromISR(xBinarySemaphore1,&xHigherPriorityTaskWoken);
		GPIO_PORTF_ICR_R |= 1<<2;
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken ); 
	}	
}
void vApplicationIdleHook(void){
}

int main(void){
		portA_init();
		portF_init();
	  portE_init();
	
	NVIC_SetPriority(mainSW_INTURRUPT_PortF,5);
	NVIC_SetPriority(mainSW_INTURRUPT_PortE,5);
	NVIC_SetPriority(mainSW_INTURRUPT_PortA,5);
 	
 	 xMutex = xSemaphoreCreateMutex();
	 vSemaphoreCreateBinary( xBinarySemaphore1);
	 vSemaphoreCreateBinary( xBinarySemaphore2); 
	 vSemaphoreCreateBinary( xBinarySemaphore3);
	 vSemaphoreCreateBinary( xBinarySemaphore4);
	 vSemaphoreCreateBinary( xBinarySemaphore5);
	 vSemaphoreCreateBinary( xBinarySemaphore6);
		
	 xTaskCreate(Lock, (const portCHAR*)"Lock", configMINIMAL_STACK_SIZE, NULL ,2,NULL);
	 xTaskCreate(PassengerUp,"PassengerUp", configMINIMAL_STACK_SIZE, NULL ,1,NULL);	
	 xTaskCreate(PassengerDown, (const portCHAR*)"PassengerDown", configMINIMAL_STACK_SIZE, NULL ,1,NULL);
	 xTaskCreate(DriverUp, (const portCHAR*)"DriverUp", configMINIMAL_STACK_SIZE, NULL ,3,NULL);
	 xTaskCreate(DriverDown, (const portCHAR*)"DriverDown", configMINIMAL_STACK_SIZE, NULL ,3,NULL);
	 xTaskCreate(Jamming, (const portCHAR*)"Jamming", configMINIMAL_STACK_SIZE, NULL ,4,NULL);
	 
	
  vTaskStartScheduler();
	
  for( ;; );
	
}



