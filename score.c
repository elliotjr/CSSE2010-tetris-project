/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include "terminalio.h"
#include <avr/pgmspace.h> // For PSTR
#include <stdio.h>
#include <stdlib.h>

// Variable to keep track of the score. We declare it as static
// to ensure that it is only visible within this module - other
// modules should call the functions below to modify/access the
// variable.
static uint32_t score;
uint16_t high_scores[5]; 
char high_score_names[5][2]; 
//static uint8_t number_of_scores = 5; 

void init_score(void) {
	score = 0;
}

void add_to_score(uint16_t value) {
	score += value;
}

uint32_t get_score(void) {
	return score;
}

void show_score_to_terminal() {
	move_cursor(20, 0);
	hide_cursor();
	printf_P(PSTR("Score: %10d"), get_score());
	move_cursor(0, 20);
} 


void save_high_score_array(void) {
	uint8_t position = 20; 
	for (uint8_t i = 0; i < 4; i++) {
		if (score > high_scores[i] || high_scores[i] == 0b1111111111111111) {
			position = i;  
			break; 
		}
	}
	for (int j = 3; j >= position; j--) {
		high_scores[j + 1] = high_scores[j]; 
	}
	
	if (position != 20) {
		high_scores[position] = score;
		eeprom_write_word(( uint16_t *)0, high_scores[0]);
		eeprom_write_word(( uint16_t *)2, high_scores[1]);
		eeprom_write_word(( uint16_t *)4, high_scores[2]);
		eeprom_write_word(( uint16_t *)6, high_scores[3]);
		eeprom_write_word(( uint16_t *)8, high_scores[4]);
		write_name_to_eeprom2(position); 
	}
	  
}


void display_high_score(void) {
	move_cursor(0,0); 
	printf_P(PSTR("HIGHSCORES\n"));
	printf_P(PSTR("__________\n"));	
	printf_P(PSTR("%c"), eeprom_read_word(( uint16_t *)100));
	printf_P(PSTR("%c: "), eeprom_read_word(( uint16_t *)102));
	printf_P(PSTR("%d\n"), eeprom_read_word(( uint16_t *)0));
	
	printf_P(PSTR("%c"), eeprom_read_word(( uint16_t *)104));
	printf_P(PSTR("%c: "), eeprom_read_word(( uint16_t *)106));
	printf_P(PSTR("%d\n"), eeprom_read_word(( uint16_t *)2));
	
	printf_P(PSTR("%c"), eeprom_read_word(( uint16_t *)108));
	printf_P(PSTR("%c: "), eeprom_read_word(( uint16_t *)110));
	printf_P(PSTR("%d\n"), eeprom_read_word(( uint16_t *)4));
	
	printf_P(PSTR("%c"), eeprom_read_word(( uint16_t *)112));
	printf_P(PSTR("%c: "), eeprom_read_word(( uint16_t *)114));
	printf_P(PSTR("%d\n"), eeprom_read_word(( uint16_t *)6));
	
	printf_P(PSTR("%c"), eeprom_read_word(( uint16_t *)116));
	printf_P(PSTR("%c: "), eeprom_read_word(( uint16_t *)118));
	printf_P(PSTR("%d\n"), eeprom_read_word(( uint16_t *)8));
}

void write_eeprom_to_game(void) {
	high_scores[0] = eeprom_read_word(( uint16_t *)0);
	high_scores[1] = eeprom_read_word(( uint16_t *)2);
	high_scores[2] = eeprom_read_word(( uint16_t *)4);
	high_scores[3] = eeprom_read_word(( uint16_t *)6);
	high_scores[4] = eeprom_read_word(( uint16_t *)8);
}

void write_eeprom_to_game_names(void) {
	high_score_names[0][0] = eeprom_read_word(( uint16_t *)100);
	high_score_names[0][1] = eeprom_read_word(( uint16_t *)102);
	
	high_score_names[1][0] = eeprom_read_word(( uint16_t *)104);
	high_score_names[1][1] = eeprom_read_word(( uint16_t *)106);
	
	high_score_names[2][0] = eeprom_read_word(( uint16_t *)108);
	high_score_names[2][1] = eeprom_read_word(( uint16_t *)110);
	
	high_score_names[3][0] = eeprom_read_word(( uint16_t *)112);
	high_score_names[3][1] = eeprom_read_word(( uint16_t *)114);
	
	high_score_names[4][0] = eeprom_read_word(( uint16_t *)116);
	high_score_names[4][1] = eeprom_read_word(( uint16_t *)118);
}

void write_name_to_eeprom2(uint8_t position) {
	if (position <= 4) {
		printf_P(PSTR("\nYou got a high score! Enter Your Name: "));
		char name[2];
		int i = 0;
		while (i < 2) {
			name[i] = fgetc(stdin);
			if ((name[i] < 65) || (name[i] > 122)) {
				continue; 
			}
			printf_P(PSTR("%c"), name[i]);
			i++;
		}
		clear_terminal();
		
		
		for (int j = 3; j >= position; j--) {
			high_score_names[j + 1][0] = high_score_names[j][0];
			high_score_names[j + 1][1] = high_score_names[j][1];
		}
		
		high_score_names[position][0] = name[0];
		high_score_names[position][1] = name[1];
		
		eeprom_write_word(( uint16_t *)100, high_score_names[0][0]);
		eeprom_write_word(( uint16_t *)102, high_score_names[0][1]);
		
		eeprom_write_word(( uint16_t *)104, high_score_names[1][0]);
		eeprom_write_word(( uint16_t *)106, high_score_names[1][1]);
		
		eeprom_write_word(( uint16_t *)108, high_score_names[2][0]);
		eeprom_write_word(( uint16_t *)110, high_score_names[2][1]);
		
		eeprom_write_word(( uint16_t *)112, high_score_names[3][0]);
		eeprom_write_word(( uint16_t *)114, high_score_names[3][1]);
		
		eeprom_write_word(( uint16_t *)116, high_score_names[4][0]);
		eeprom_write_word(( uint16_t *)118, high_score_names[4][1]);
		
	}
}

/*void write_name_to_eeprom(uint8_t position) {
	//If the person has made a new highscore
	if (position <= 4) {
		printf_P(PSTR("\nYou got a high score! Enter Your Name: "));
		char name[2];
		int i = 0;
		while (i < 2) {
			name[i] = fgetc(stdin);
			printf_P(PSTR("%c"), name[i]);
			i++;
		}
		clear_terminal();
		if (position == 0) {
			eeprom_write_word(( uint16_t *)100, name[0]);
			eeprom_write_word(( uint16_t *)102, name[1]);
			} else if (position == 1) {
			eeprom_write_word(( uint16_t *)104, name[0]);
			eeprom_write_word(( uint16_t *)106, name[1]);
			} else if (position == 2) {
			eeprom_write_word(( uint16_t *)108, name[0]);
			eeprom_write_word(( uint16_t *)110, name[1]);
			} else if (position == 3) {
			eeprom_write_word(( uint16_t *)112, name[0]);
			eeprom_write_word(( uint16_t *)114, name[1]);
			} else if (position == 4) {
			eeprom_write_word(( uint16_t *)116, name[0]);
			eeprom_write_word(( uint16_t *)118, name[1]);
			} else {
		}	
	}	
}
*/