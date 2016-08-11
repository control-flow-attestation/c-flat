/*
 * LCD Display API
 */

#ifndef LIQUID_CRYSTAL_H
#define LIQUID_CRYSTAL_H

typedef struct LiquidCrystalStruct{
	unsigned int id;
} LiquidCrystal;

void lcd_begin(LiquidCrystal* lcd, unsigned int cols, unsigned int rows);
void lcd_clear(LiquidCrystal* lcd); //http://stackoverflow.com/questions/10105666/clearing-the-terminal-screen
void lcd_print(LiquidCrystal* lcd, char* output, int len);
void lcd_setCursor(LiquidCrystal* lcd, int x, int y); //http://stackoverflow.com/questions/10105666/clearing-the-terminal-screen

#endif /* LIQUID_CRYSTAL_H */
