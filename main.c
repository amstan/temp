#include <msp430.h>

#include "bitop.h"
#include "debug.h"

#include <stdint.h>
#include "config.h"
#include "ringbuffer.h"
#include "usci_serial.h"

ringbuffer_ui8_16 usci_buffer = { 0, 0, { 0 } };
Serial<ringbuffer_ui8_16> usci0 = { usci_buffer };
void __attribute__((interrupt (USCIAB0RX_VECTOR))) USCI0RX_ISR() {
	usci_buffer.push_back(UCA0RXBUF);
}

void __delay_ms(unsigned int ms) {
	for(;ms!=0;ms--) {
		__delay_cycles(16000);
	}
}

void chip_init(void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
	DCOCTL  = CALDCO_16MHZ; // Load the clock calibration
	BCSCTL1 = CALBC1_16MHZ;
}

void io_init(void) {
	#define LED_R 0
	#define LED_G 6
	#define SW 3
	
	P1DIR=0b01000001;
	P1OUT=0b00000000;
}

void temp_init(void) {
	ADC10CTL0 = 0;
	ADC10CTL1 = 0;
	
	ADC10CTL1 |= INCH_10; //temperature channel
	ADC10CTL1 |= ADC10DIV_7; //8x clock divider
	
	ADC10CTL0 |= SREF_1; //voltage reference
	ADC10CTL0 |= ADC10SHT_3; //64x sample & hold
	ADC10CTL0 |= REFON; //enable reference
	ADC10CTL0 |= ADC10ON; //enable adc
	ADC10CTL0 |= ADC10IE; //enable ADC conversion complete interrupt
}

int main(void) {
	
	chip_init();
	io_init();
	temp_init();
	usci0.init();
	set_bit(P1OUT,LED_G);
	
	while(1) {
		//wait until computer asks for temperature
		usci0.recv();
		
		set_bit(P1OUT,LED_R); //turn on red led
		
		unsigned int average=0; //16 bit
		#define AVERAGECOUNT 128
		for(unsigned int i = 0; i<AVERAGECOUNT; i++) {
			ADC10CTL0 |= (ENC | ADC10SC); //begin ADC conversion
			__bis_SR_register(LPM2_bits + GIE); //sleep until conversion complete
			
			unsigned int ADC_Result;
			ADC_Result = ADC10MEM;
			ADC_Result -= 512; //we don't care about those values(under -65C), so ADC_Result is now essentially 9 bit
			average += ADC10MEM;
			
			__delay_cycles(100);
		}
		
		clear_bit(P1OUT,LED_R); //turn off red led
		
		//reply to the computer
		usci0.xmit(average/256);
		usci0.xmit(average%256);
	}
}

void __attribute__((interrupt (ADC10_VECTOR))) adc_complete(void)
{
    __bic_SR_register_on_exit(LPM0_bits); //Wakeup main code
}