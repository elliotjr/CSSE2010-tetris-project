/*
 * timer0.c
 *
 * Author: Peter Sutton
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"

/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clock_ticks;


void mute_timer(void) {
	TCCR2B |= (1 << CS22) | (1 << CS21) | (CS20); 
}

void clear_sound(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clock_ticks = 0L;
	DDRD |= (1 << DDRD7) | (0<<DDRD6); 
	
	/* Clear the timer */
	TCNT2 = 0;

	/* Set the output compare value to be 124 */
	OCR2A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR2A = (1<<WGM21) | (1 << COM2A0);
	TCCR2B = (1<<CS21)|(0<<CS20)|(1<<CS22);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK2 |= (1<<OCIE2A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR2 &= (1<<OCF2A);
}


void rotate_sound(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clock_ticks = 0L;
	DDRD |= (1 << DDRD7) | (0<<DDRD6); 
	
	/* Clear the timer */
	TCNT2 = 0;

	/* Set the output compare value to be 124 */
	OCR2A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR2A = (1<<WGM21) | (1 << COM2A0);
	TCCR2B = (1<<CS21)|(0<<CS20);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK2 |= (1<<OCIE2A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR2 &= (1<<OCF2A);
}

void init_timer2(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clock_ticks = 0L;
	DDRD |= (1 << DDRD7) | (0<<DDRD6); 
	
	/* Clear the timer */
	TCNT2 = 0;

	/* Set the output compare value to be 124 */
	OCR2A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR2A = (1<<WGM21) | (1 << COM2A0);
	TCCR2B = (1<<CS21)|(1<<CS20);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK2 |= (1<<OCIE2A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR2 &= (1<<OCF2A);
}

/* Interrupt handler which fires when timer/counter 0 reaches 
 * the defined output compare value (every millisecond)
 */
ISR(TIMER2_COMPA_vect) {
	if (clock_ticks < 300) {
		clock_ticks ++; 
	} else {
		TCCR2A |= (1 << COM2A1) | (1 << COM2A0); 
	}
}
