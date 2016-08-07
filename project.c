/*
 * project.c
 *
 * Main file for the Tetris Project.
 *
 * Author: Peter Sutton. Modified by Elliot Randall
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>		// For random()
#include <inttypes.h>   //for printing uint32_t

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "game.h"
#include "timer2.h"
#include "joystick.h"

#define F_CPU 8000000L
#include <util/delay.h>
#include <avr/eeprom.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void handle_new_lap(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27 

int press_sequence = 0; 

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	//write the highscores to the game
	write_eeprom_to_game(); 
	write_eeprom_to_game_names();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	DDRC = 0xFF;
	// PORT7 for common cathode output, PIN0 & PIN1 for ADC
	DDRA = (1 << DDRA7) | (0 << DDRA0) | (0 << DDRA1);
	
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	//input for mute button
	
	// Set up our main timer to give us an interrupt every millisecond
	init_timer0();
	//write_eeprom_to_game();
	//Set up sound Timer
	
	// Turn on global interrupts
	sei();
}

void splash_screen(void) {
	// Reset display attributes and clear terminal screen then output a message
	set_display_attribute(TERM_RESET);
	clear_terminal();
	
	hide_cursor();	// We don't need to see the cursor when we're just doing output
	display_high_score();
	move_cursor(10,10);
	printf_P(PSTR("s4356917"));
	
	move_cursor(10,13);
	set_display_attribute(FG_GREEN);	// Make the text green
	printf_P(PSTR("CSSE2010/7201 Tetris Project by Elliot Randall"));	
	set_display_attribute(FG_WHITE);	// Return to default colour (White)
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	
	// Red message the first time through
	PixelColour colour = COLOUR_RED; 
	while(1) { 
		set_scrolling_display_text("s4356917", colour);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != -1) {
				// A button has been pushed
				return;
			}
		}
		// Message has scrolled off the display. Change colour
		// to a random colour and scroll again.
		switch(random()%4) {
			case 0: colour = COLOUR_LIGHT_ORANGE; break;
			case 1: colour = COLOUR_RED; break;
			case 2: colour = COLOUR_YELLOW; break;
			case 3: colour = COLOUR_LIGHT_GREEN; break;
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	init_game();
	
	// Clear the serial terminal
	//clear_terminal();
	
	// Initialise the score
	init_score();
	reset_current_speed(); 
	
	// Delete any pending button pushes or serial input
	empty_button_queue();
	clear_serial_input_buffer();
}

void play_game(void) {
	init_joystick();
	uint16_t adc_value = 511; 
	int currentSpeed = 600; 
	uint32_t last_drop_time;
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	//uint8_t press_sequence = 0; //If the joystick has been pressed and is being held down
	
	// Record the last time a block was dropped as the current time -
	// this ensures we don't drop a block immediately.
	last_drop_time = get_clock_ticks();
	//uint8_t in_loop = 0; 
	if (PIND & (1 << PIND6)) {
		//Mute
	} else {
			clear_sound();
			_delay_ms(200);
			rotate_sound();
			_delay_ms(200);
			init_timer2();
	}
	set_is_running();
	// We play the game forever. If the game is over, we will break out of
	// this loop. The loop checks for events (button pushes, serial input etc.)
	// and on a regular basis will drop the falling block down by one row.
	while(1) { 
		if (get_value(0) > 900 || get_value(0) < 200 || get_value(1) > 900 || get_value(1) < 200 ) {
			press_sequence = 1; 
		} else {
			press_sequence = 0; 
		}
	
		show_score_to_terminal();
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. We will be processing each character
		// independently and can't do anything until we get the third character.
		// At most one of the following three variables will be set to a value 
		// other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// 
		//pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop 
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		
		if(button == -1) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input
		uint32_t current_clock = get_clock_ticks(); 
		if(button==3 || escape_sequence_char=='D' || ((adc_value = get_value(0))) < 200) {
			//get the current clock   | current_clock = get_clock_ticks();
			//check if it was the ADC | if (adc_value  < 200)
			//wait 300ms              | while (get_clock_ticks() < current_clock + 300) {}
			//then move left
			current_clock = get_clock_ticks(); 
			if ((adc_value < 200) && press_sequence == 0) {
				(void)attempt_move(MOVE_LEFT); 
				while (get_clock_ticks() < current_clock + 300) {
					//Do Nothing
				}
			} else if ((adc_value < 200) && press_sequence == 1){
				current_clock = get_clock_ticks(); 
				(void)attempt_move(MOVE_LEFT);
				while (get_clock_ticks() < current_clock + 100) {
					//Do Nothing
				}
			} else { //Don't delay if not joystick
				(void)attempt_move(MOVE_LEFT);
			}
		} else if(button==0 || escape_sequence_char=='C' || ((adc_value = get_value(0))) > 900) {
			/*
			uint32_t current_clock = get_clock_ticks();
			if (adc_value > 900) {
				(void)attempt_move(MOVE_RIGHT);
				while (get_clock_ticks() < current_clock + 300 ) {
					//Do Nothing
				}
				} else { //Don't delay if not joystick
				(void)attempt_move(MOVE_RIGHT);
			}
			*/
			//___________________________________________________________________
			current_clock = get_clock_ticks();
			if ((adc_value > 900) && press_sequence == 0) {
				(void)attempt_move(MOVE_RIGHT);
				while (get_clock_ticks() < current_clock + 300) {
					//Do Nothing
				}
				} else if ((adc_value > 900) && press_sequence == 1){
				current_clock = get_clock_ticks();
				(void)attempt_move(MOVE_RIGHT);
				while (get_clock_ticks() < current_clock + 100) {
					//Do Nothing
				}
				} else { //Don't delay if not joystick
				(void)attempt_move(MOVE_RIGHT);
			}
			//____________________________________________________________________
			
		} else if (button==2 || escape_sequence_char == 'A' || ((adc_value = get_value(1))) > 900) {
			// Attempt to rotate
			//(void)attempt_rotation();
			
			/*uint32_t current_clock = get_clock_ticks();
			if (adc_value > 900) {
				(void)attempt_rotation();
				while (get_clock_ticks() < current_clock + 300) {
					//Do Nothing
				}
				} else { //Don't delay if not joystick
				(void)attempt_rotation();
			}*/
			
			if (PIND & (1 << PIND6)) {
			} else {
				rotate_sound();
			}
			
			current_clock = get_clock_ticks();
			if ((adc_value > 900) && press_sequence == 0) {
				(void)attempt_rotation();
				while (get_clock_ticks() < current_clock + 300) {
					//Do Nothing
				}
				} else if ((adc_value > 900) && press_sequence == 1){
				current_clock = get_clock_ticks();
				(void)attempt_rotation();
				while (get_clock_ticks() < current_clock + 100) {
					//Do Nothing
				}
				} else { //Don't delay if not joystick
				(void)attempt_rotation();
			}
			
		} else if (button == 1 || serial_input == ' ') {
				if (PIND & (1 << PIND6)) {
					} else {
					init_timer2();
				} 	
			while(attempt_drop_block_one_row());
			
			
		} else if (escape_sequence_char == 'B' || ((adc_value = get_value(1)) < 100)) {
			/*// Attempt to drop block
			//while(attempt_drop_block_one_row()); 
			if(!attempt_drop_block_one_row()) {
				// Drop failed - fix block to board and add new block
				if(!fix_block_to_board_and_add_new_block()) {
					break;	// GAME OVER
				}
			} 
			last_drop_time = get_clock_ticks();*/
			
			//_________________________________________
			current_clock = get_clock_ticks();
			if ((adc_value < 100) && press_sequence == 0) {
				if(!attempt_drop_block_one_row()) {
					// Drop failed - fix block to board and add new block
					if(!fix_block_to_board_and_add_new_block()) {
						break;	// GAME OVER
					}
				}
				last_drop_time = get_clock_ticks();
				while (get_clock_ticks() < current_clock + 300) {
					//Do Nothing
				}
				} else if ((adc_value < 100) && press_sequence == 1){
				current_clock = get_clock_ticks();
				if(!attempt_drop_block_one_row()) {
					// Drop failed - fix block to board and add new block
					if(!fix_block_to_board_and_add_new_block()) {
						break;	// GAME OVER
					}
				}
				last_drop_time = get_clock_ticks();
				while (get_clock_ticks() < current_clock + 100) {
					//Do Nothing
				}
				} else { //Don't delay if not joystick
				if(!attempt_drop_block_one_row()) {
					// Drop failed - fix block to board and add new block
					if(!fix_block_to_board_and_add_new_block()) {
						break;	// GAME OVER
					}
				}
				last_drop_time = get_clock_ticks();
			}
			
			
			
			
			
			
			
			//_________________________________________
			
			
			
			
		} else if(serial_input == 'p' || serial_input == 'P') { 
			empty_button_queue();
			///my implementation///
			uint32_t time_to_wait;
			uint32_t current_pause = get_clock_ticks(); 
			while(1) {
				time_to_wait = get_clock_ticks() - last_drop_time; 
				move_cursor(10, 14);
				printf_P(PSTR("%s"), get_score());
				serial_input = fgetc(stdin); 
				if(serial_input == 'p' || serial_input == 'P') {
					break;
				}
				
			}
			current_pause = get_clock_ticks(); 
			while(get_clock_ticks() < current_pause + time_to_wait){
				//Do Nothing
			}
			//clear_terminal();
			empty_button_queue();
		} 
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		// Check for timer related events here
		if(get_clock_ticks() >= last_drop_time + currentSpeed) {
			//accelerate when a row is cleared
			if (currentSpeed < 200) {
				currentSpeed = currentSpeed; 
			} else {
				currentSpeed = get_acceleration();//currentSpeed * get_acceleration(); 
 
				
			}
			// 600ms (0.6 second) has passed since the last time we dropped
			// a block, so drop it now.
			if(!attempt_drop_block_one_row()) {
				// Drop failed - fix block to board and add new block
				if(!fix_block_to_board_and_add_new_block()) {
					break;	// GAME OVER
				}
			}
			last_drop_time = get_clock_ticks();
		}
	}
	// If we get here the game is over. 
}


void handle_game_over() {
	clear_terminal();
	move_cursor(10,14);
	// Print a message to the terminal. 
	printf_P(PSTR("GAME OVER"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	move_cursor(10,16);
	printf_P(PSTR("\nScore: %10d"), get_score());
	save_high_score_array();
	empty_button_queue();
	_delay_ms(10); 
	move_cursor(10,14);
	set_display_attribute(FG_CYAN);
	printf_P(PSTR("\n\n")); 
	display_high_score();
	
	
	
	
	
	//We now have the users name stored in an array - this needs to be written to EEPROM in the suitable position
	//store_name_in_array();
	
	
	normal_display_mode();
	while(button_pushed() == -1) {
		; // wait until a button has been pushed
	}
	//reset the cleared rows counter to 0
	set_cleared_count(0); 
}