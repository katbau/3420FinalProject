#define ADC_CHANNEL 12 // Channel 12 (PTB2)
#define LED_BLUE  21 // PTB21
#define  ADC_READS 8
uint16_t  value[ADC_READS];

void adc_init(void);
int adc_cal(void);
unsigned short adc_read(unsigned char ch);
void dma_init(void);
void pit_init(void);
void PIT0_IRQHandler(void);
int test_joystick(void);