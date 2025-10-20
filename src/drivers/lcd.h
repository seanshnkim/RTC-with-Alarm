/**
 * @file lcd.h
 * @brief LCD Display Driver for STM32F429I Discovery Board
 * @author Generated for STM32F429I Discovery LCD
 * @date 2025
 */

#ifndef LCD_H
#define LCD_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// LCD Configuration
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// LCD Pin definitions using manual GPIO control (STM32F429I-Discovery)
// ILI9341 is wired to SPI5 pins and control lines on the Discovery board.
// CS  -> PC2,  DC -> PD13,  RST -> PF10,  SCK -> PF7 (SPI5_SCK),  MOSI -> PF9 (SPI5_MOSI)
#define LCD_CS_PIN         GPIO_PIN_2
#define LCD_CS_GPIO_PORT   GPIOC
#define LCD_DC_PIN         GPIO_PIN_13
#define LCD_DC_GPIO_PORT   GPIOD
#define LCD_RST_PIN        GPIO_PIN_10
#define LCD_RST_GPIO_PORT  GPIOF
#define LCD_SCK_PIN        GPIO_PIN_7
#define LCD_SCK_GPIO_PORT  GPIOF
#define LCD_MOSI_PIN       GPIO_PIN_9
#define LCD_MOSI_GPIO_PORT GPIOF

// Colors (RGB565 format)
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

// Function prototypes
void MX_LCD_GPIO_Init(void);
void LCD_Init(void);
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_Clear(uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color);
void LCD_DrawString(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bg_color);
void LCD_PrintTask(uint16_t x, uint16_t y, char* message, uint16_t color);
void Manual_SPI_Transmit(uint8_t data);

#endif /* LCD_H */