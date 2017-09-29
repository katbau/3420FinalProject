#include "MK64F12.h"
#include "joystick.h"


void adc_init(void)
{
    // Enable clocks
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;  //Enable clok ADC 0 clock
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; // PTB0 clock
    
    // Calibrate ADC
    adc_cal();

    // Configure ADC
    ADC0->CFG1 = 0; // Reset register
    ADC0->CFG1 |= (ADC_CFG1_MODE(3)  |      // 16 bits mode
                  ADC_CFG1_ADICLK(0)|   // Input Bus Clock (20-25 MHz out of reset (FEI mode))
                  ADC_CFG1_ADIV(1)) ;   // Clock divide by 2 (10-12.5 MHz)
    
    ADC0->SC2 |= ADC_SC2_DMAEN_MASK;    // DMA Enable
    
    ADC0->SC3 = 0; // Reset SC3
    
    ADC0_SC1A |= ADC_SC1_ADCH(31); // Disable module
}

/* adc_cal
 * Calibrates the adc
 * Returns 0 if successful calibration
 * Returns 1 otherwise
 * */
int adc_cal(void)
{
    ADC0->CFG1 |= (ADC_CFG1_MODE(3)  |      // 16 bits mode
                  ADC_CFG1_ADICLK(1)|   // Input Bus Clock divided by 2 (20-25 MHz out of reset (FEI mode) / 2)
                  ADC_CFG1_ADIV(2)) ;   // Clock divide by 4 (2.5-3 MHz)
    
    ADC0->SC3 |= ADC_SC3_AVGE_MASK |        // Enable HW average
                ADC_SC3_AVGS(3)   |     // Set HW average of 32 samples
                ADC_SC3_CAL_MASK;       // Start calibration process
    
    while(ADC0->SC3 & ADC_SC3_CAL_MASK); // Wait for calibration to end
    
    if(ADC0->SC3 & ADC_SC3_CALF_MASK)   // Check for successful calibration
        return 1; 
    
    uint16_t calib = 0; // calibration variable 
    calib += ADC0->CLPS + ADC0->CLP4 + ADC0->CLP3 +
             ADC0->CLP2 + ADC0->CLP1 + ADC0->CLP0;
    calib /= 2;
    calib |= 0x8000;    // Set MSB 
    ADC0->PG = calib;
    calib = 0;
    calib += ADC0->CLMS + ADC0->CLM4 + ADC0->CLM3 +
             ADC0->CLM2 + ADC0->CLM1 + ADC0->CLM0;
    calib /= 2;
    calib |= 0x8000;    // Set MSB
    ADC0->MG = calib;
    
    return 0;
}

/*unsigned short    adc_read(unsigned char ch)
 *  Reads the specified adc channel and returns the 16 bits read value
 *  
 *  ch -> Number of the channel in which the reading will be performed
 *  Returns the -> Result of the conversion performed by the adc
 * 
 * */
unsigned short adc_read(unsigned char ch)
{
    ADC0_SC1A = (ch & ADC_SC1_ADCH_MASK) |
                (ADC0_SC1A & (ADC_SC1_AIEN_MASK | ADC_SC1_DIFF_MASK));     // Write to SC1A to start conversion
    while(ADC0_SC2 & ADC_SC2_ADACT_MASK);    // Conversion in progress
    while(!(ADC0_SC1A & ADC_SC1_COCO_MASK)); // Run until the conversion is complete
    return ADC0->R[1];
}
void dma_init(void)
{
    // Enable clock for DMAMUX and DMA
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
    SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;    
            
    // Enable Channel 0 and set ADC0 as DMA request source 
    DMAMUX_CHCFG0 |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(40);

    // Enable request signal for channel 0 
    DMA_ERQ = DMA_ERQ_ERQ0_MASK;
        
    // Set memory address for source and destination 
    DMA_TCD0_SADDR = (uint32_t)&ADC0_RA;
    DMA_TCD0_DADDR = (uint32_t)&value;

    // Set an offset for source and destination address
    DMA_TCD0_SOFF = 0x00; // Source address offset of 2 bits per transaction
    DMA_TCD0_DOFF = 0x02; // Destination address offset of 1 bit per transaction
        
    // Set source and destination data transfer size
    DMA_TCD0_ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);
        
    // Number of bytes to be transfered in each service request of the channel
    DMA_TCD0_NBYTES_MLNO = 0x02;
        
    // Current major iteration count (a single iteration of 5 bytes)
    DMA_TCD0_CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(ADC_READS);
    DMA_TCD0_BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(ADC_READS);
    
    // Adjustment value used to restore the source and destiny address to the initial value
    DMA_TCD0_SLAST = 0x00;      // Source address adjustment
    DMA_TCD0_DLASTSGA = -0x10;  // Destination address adjustment
    
    // Setup control and status register
    DMA_TCD0_CSR = 0;
}


/* Initializes the PIT module to produce an interrupt every second
 * 
 * */
void pit_init(void)
{
    // Enable PIT clock
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    
    // Enable Blue LED clock and MUX
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    PORTB->PCR[21] = PORT_PCR_MUX(1);
    GPIOB_PDDR |= (1 << LED_BLUE);
    GPIOB_PSOR |= (1 << LED_BLUE);
    
    // Turn on PIT
    PIT->MCR = 0;
    
    // Configure PIT to produce an interrupt every 1s
    PIT->CHANNEL[0].LDVAL = 0x1312CFF;  // 1/20Mhz = 50ns   (1s/50ns)-1= 19,999,999 cycles or 0x1312CFF
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK; // Enable interrupt and enable timer
    
    // Enable interrupt registers ISER and ICPR
    NVIC_EnableIRQ(PIT0_IRQn);
}

/*  Handles PIT interrupt if enabled
 * 
 *  Starts conversion in ADC0 with single ended channel 8 (PTB0) as input
 * 
 * */
void PIT0_IRQHandler(void)
{   
    // Clear interrupt
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    
    // Write to SC1A to start conversion with channel 12 PTB2
    ADC0_SC1A = (ADC_SC1_ADCH(ADC_CHANNEL) |
                 (ADC0_SC1A & (ADC_SC1_AIEN_MASK | ADC_SC1_DIFF_MASK)));  
    unsigned short voltage =  adc_read(ADC_CHANNEL);
    // Toggle Blue LED
    if (voltage > 1){
        LEDRed_On();
    }
    else{
        LEDRed_Off();
    }    
}

int test_joystick(void)
{
    adc_init();
    pit_init();
    dma_init();
    return 0;
}