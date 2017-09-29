#include <fsl_device_registers.h>


unsigned int counter = 0;
int led_on = 0;

//Helper functions to turn LEDs on/off
void LedBlue_On(){
	PTB->PCOR = 1 << 21; //Turn blue LED on
}
void LedBlue_Off(){
	PTB->PSOR = 1 << 21; //Turn blue LED off
}
void LedGreen_On(){
	PTE->PCOR = 1 << 26; //Turn green LED on
}
void LedGreen_Off(){
	PTE->PSOR = 1 << 26; //Turn green LED off
}

int joystick_interrupt(void)
{
	NVIC_EnableIRQ(PIT0_IRQn); /* enable PIT0 Interrupts (for part 3) */

	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; //Enable the clock to port B
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; //Enable the clock to port E
	
	PORTB->PCR[21] = PORT_PCR_MUX(001); //Set up PTB21 as GPIO
	PORTE->PCR[26] = PORT_PCR_MUX(001); //Set up PTE26 as GPIO

	PORTB->PCR[2] = (1 << 8); 	/* Pin PTB2  is GPIO (used for Sw)*/
  	PORTB->PCR[3] = (1 << 8); 	/* Pin PTB3  is GPIO (used for x)*/
  	PORTB->PCR[10] = (1 << 8); 	/* Pin PTB10  is GPIO (used for y)*/

	PTB->PDDR = 0 << 2;//Enable PTB2 (SW) as input
	PTB->PDDR = 0 << 3; //Enable PTB3 (x) as input
	PTB->PDDR = 0 << 10; //Enable PTB10 (y) as input
	PTB->PDDR = 1 << 21; //Enable PTB21 (blue) as output
	PTE->PDDR = 1 << 26; //Enable PTB21 (green) as output

	PTB->PSOR = 1 << 21; //Turn blue LED off
	PTE->PSOR = 1 << 26; //Turn green LED off
	
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable clock to PIT module
	PIT->MCR = 0x0;
	PIT->CHANNEL[0].LDVAL = 0x1400000; //Set timer value to ~1sec (=20971520)
	PIT->CHANNEL[0].TFLG = PIT->CHANNEL[0].TFLG | 1; //Clear flag before enabling interrupts
	PIT->CHANNEL[0].TCTRL = 0x3; //Set TCTRL to 3 to enable timer + interrupts
	
	while(1){ //Toggle blue LED ~once a second
		LedBlue_On(); //Turn blue LED on
		for (counter = 0x14CCC4; counter > 0; counter--) {;}
		LedBlue_Off(); //Turn blue LED off
		for (counter = 0x14CCC4; counter > 0; counter--) {;}
	}
}

void LedGreen_Toggle(void){
     if (led_on == 0){
        LedGreen_On(); //Turn green LED on
		PIT->CHANNEL[0].TCTRL = 0x2; //Disable timer to reload timer value
        PIT->CHANNEL[0].LDVAL = 0x200000; //Set timer value to ~1/10th sec (=2097152)
		PIT->CHANNEL[0].TCTRL = 0x3; //Reenable timer
		led_on = 1;
    }else{
        LedGreen_Off(); //Turn green LED off
		PIT->CHANNEL[0].TCTRL = 0x2; //Disable timer to reload timer value
        PIT->CHANNEL[0].LDVAL = 0x1400000; //Set timer value to ~1sec (=20971520)
		PIT->CHANNEL[0].TCTRL = 0x3; //Reenable timer
		led_on = 0;
    }
}

/* 
     PIT Interrupt Handler
*/
void PIT0_IRQHandler(void)
{
	if (PDIR & (1 << 2) == 1){
		//if SW value is set to 1 then toggle green led
		LedGreen_Toggle();
	}
	PIT->CHANNEL[0].TFLG = PIT->CHANNEL[0].TFLG | 1; //Clear interrupt flag
}
