/*
 * lcd_i2c.h
 *
 *  Created on: 24-Mar-2026
 *      Author: POOBALASHREE
 */

#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_


#include "stm32f4xx_hal.h"

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_Send_String(char *str);
void LCD_Set_Cursor(uint8_t row, uint8_t col);
void LCD_Clear(void);


#endif /* INC_LCD_I2C_H_ */
