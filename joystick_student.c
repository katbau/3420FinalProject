#include <fsl_device_registers.h>

void ADC_init(void){
	//NOTES ADC0_SC1A == PTB3 == y axis
	//		ADC0_SE12 == PTB2
	//		ADC1_SE14 == PTB10
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK; 	// Enable clock to the ADC
	ADC0_CFG1 |= ADC_CFG1_MODE(3); 		// Sets the MODE bits of the ADC0_CFG1 register to "11", which is 3 in decimal. This sets the ADC to 16-bit mode. (see the reference manual "ADCx_CFG1 ")
	//ADC1_CFG1 |= ADC_CFG1_MODE(3);      // 16 bit mode
	//turn off all the important pins for ADC
	ADC0_SC1A |= ADC_SC1_ADCH(31); 		// Set the ADCH bits of the ADC0_SC1A register to 11111, which means the ADC is turned off (see the refernce manual "ADCx_SC1n")
	//ADC0_SE12 |= ADC_SE1_ADCH(31);
	//ADC1_SE14 |= ADC_SE1_ADCH(31);
	while(ADC0_SC2 & ADC_SC2_ADACT_MASK); 	// Checks the ADACT bit of the ADC0_SC2 register, which is 1 while a conversion is in progress. (see reference manual "ADCx_SC2").
    //while(ADC1_SC2 & ADC_SC2_ADACT_MASK); 	
    while(!(ADC0_SC1A & ADC_SC1_COCO_MASK));    // Checks the COCO bit of the ADC0_SC1A register, which is 1 when the conversion is complete. (see reference manual "ADCx_SCAn")
    return ADC0_RA;				// Returns the data result register (see reference manual "ADCx_Rn")
}
unsigned short ADC_read16b(void)
{
	ADC0_SC1A = 12 & ADC_SC1_ADCH_MASK; 	// Sets to ADCH bits of ADC0_SCA1 to 01100, which selects pin 12 (I think this is PTB2 but I am not sure) as the input (01100 = 12 in decimal). Enables the ADC (see referece manual "ADCx_SCAn").
	while(ADC0_SC2 & ADC_SC2_ADACT_MASK); 	// Checks the ADACT bit of the ADC0_SC2 register, which is 1 while a conversion is in progress. (see reference manual "ADCx_SC2").
    while(!(ADC0_SC1A & ADC_SC1_COCO_MASK));    // Checks the COCO bit of the ADC0_SC1A register, which is 1 when the conversion is complete. (see reference manual "ADCx_SCAn")
    return ADC0_RA;				// Returns the data result register (see reference manual "ADCx_Rn")
}

void joystick_test(void){
	unsigned short voltage =  ADC_read16b(ADC_CHANNEL);
    // Toggle Blue LED
    if (voltage > 1){
        LEDRed_On();
    }
    else{
        LEDRed_Off();
    }    
}
