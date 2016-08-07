/*
 * score.h
 * 
 * Author: Peter Sutton
 */

#ifndef SCORE_H_
#define SCORE_H_

#include <stdint.h>

void init_score(void);
void save_high_score_array(void);
void add_to_score(uint16_t value);
uint32_t get_score(void);
void show_score_to_terminal();
void display_high_score(void);
void write_eeprom_to_game(void);
void write_name_to_eeprom(uint8_t position); 



void write_eeprom_to_game_names(void);
void write_name_to_eeprom2(uint8_t position);

#endif /* SCORE_H_ */