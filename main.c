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
// 	ADC10CTL0 |= ADC10SHT_2 + ADC10ON;        // ADC10ON, S&H=16 ADC clks
// 	ADC10CTL1 |= ADC10SHP;                    // ADCCLK = MODOSC; sampling timer
// 	ADC10CTL2 |= ADC10RES;                    // 10-bit conversion results
// 	ADC10MCTL0 |= ADC10INCH_13;                // A1 ADC input select; Vref=AVCC
// 	ADC10IE |= ADC10IE0;                      // Enable ADC conv complete interrupt
	
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + REFOUT + ADC10ON + ADC10IE;
	ADC10CTL1 = INCH_10 + ADC10DIV_7;
	//set_bit(ADC10AE0,4);
}

int main(void) {
	chip_init();
	io_init();
	temp_init();
	
	set_bit(P1OUT,LED_G);
	
	unsigned int ADC_Result;
	
	usci0.init();
	while(1) {
		usci0.recv();
		set_bit(P1OUT,LED_R);
		
		//ADC_Result=100;
		
		//ADC10CTL0 |= ADC10ENC + ADC10SC;        // Sampling and conversion start
		ADC10CTL0 |= ENC + ADC10SC;
		
		while (ADC10CTL1 & BUSY);
		//__delay_ms(100);
		clear_bit(P1OUT,LED_R);
		ADC_Result = ADC10MEM;
		
		usci0.xmit(ADC_Result/256);
		usci0.xmit(ADC_Result%256);
	}
}
