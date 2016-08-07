/*
 * joystick.c
 *
 * Created: 5/23/2016 3:42:13 PM
 *  Author: Elliot
 */ 

#include <avr/io.h>


void init_joystick(void) {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADPS1) | (1 << ADPS2) | (1 << ADEN);
}

//this method takes a 0 or 1 - 0 for x, 1 for y
uint16_t get_value(uint8_t x_or_y) {
	uint16_t value;
	// Set the ADC mux to choose ADC0 if x_or_y is 0, ADC1 if x_or_y is 1
	if(x_or_y == 0) {
		ADMUX &= ~(1 << MUX0);
		} else {
		ADMUX |= (1 << MUX0);
	}
	//Start the conversion
	ADCSRA |= (1 << ADSC);
		
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	value = ADC;
	return value; 
}


