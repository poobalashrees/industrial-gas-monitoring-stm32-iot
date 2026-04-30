/*
 * lcd_i2c.c
 *
 *  Created on: 24-Mar-2026
 *      Author: POOBALASHREE
 */
#include "lcd_i2c.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define LCD_ADDR 0x4E  // try 0x4E or 0x7E if not working

void LCD_Send_Command(char cmd)
{
    uint8_t data_u, data_l;
    uint8_t data_t[4];

    data_u = (cmd & 0xF0);
    data_l = ((cmd<<4) & 0xF0);

    data_t[0] = data_u|0x0C;
    data_t[1] = data_u|0x08;
    data_t[2] = data_l|0x0C;
    data_t[3] = data_l|0x08;

    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_t, 4, 100);
}

void LCD_Send_Data(char data)
{
    uint8_t data_u, data_l;
    uint8_t data_t[4];

    data_u = (data & 0xF0);
    data_l = ((data<<4) & 0xF0);

    data_t[0] = data_u|0x0D;
    data_t[1] = data_u|0x09;
    data_t[2] = data_l|0x0D;
    data_t[3] = data_l|0x09;

    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_t, 4, 100);
}

void LCD_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_Delay(50);
    LCD_Send_Command(0x02);
    LCD_Send_Command(0x28);
    LCD_Send_Command(0x0C);
    LCD_Send_Command(0x06);
    LCD_Send_Command(0x01);
    HAL_Delay(5);
}

void LCD_Send_String(char *str)
{
    while(*str) LCD_Send_Data(*str++);
}

void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    uint8_t pos = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_Send_Command(pos);
}

void LCD_Clear(void)
{
    LCD_Send_Command(0x01);
    HAL_Delay(2);
}

