#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

SPI_HandleTypeDef hspi5;
uint8_t clock_source = 0; // 0 = HSE, 1 = HSI

// LCD Configuration
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// LCD Pin definitions for STM32F429I Discovery
#define LCD_CS_PIN       GPIO_PIN_2
#define LCD_CS_GPIO_PORT GPIOC
#define LCD_DC_PIN       GPIO_PIN_13
#define LCD_DC_GPIO_PORT GPIOD

// Colors (RGB565 format)
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_SPI5_Init(void);
void LCD_Init(void);
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_Clear(uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color);
void LCD_DrawString(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bg_color);
void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI5_Init();
    LCD_Init();
    
    // Clear screen to black
    LCD_Clear(COLOR_BLACK);
    
    // Display startup message
    LCD_DrawString(10, 10, "STM32F429I Discovery", COLOR_WHITE, COLOR_BLACK);
    LCD_DrawString(10, 30, "LCD Debug Display", COLOR_YELLOW, COLOR_BLACK);
    
    // Display clock source information
    char clock_msg[50];
    if (clock_source == 0) {
        sprintf(clock_msg, "Clock: HSE (External)");
        LCD_DrawString(10, 50, clock_msg, COLOR_GREEN, COLOR_BLACK);
    } else {
        sprintf(clock_msg, "Clock: HSI (Internal)");
        LCD_DrawString(10, 50, clock_msg, COLOR_BLUE, COLOR_BLACK);
    }
    
    // Display test messages
    LCD_DrawString(10, 80, "Test 1: HELLO", COLOR_WHITE, COLOR_BLACK);
    LCD_DrawString(10, 100, "Test 2: ABCDEFGHIJKLM", COLOR_WHITE, COLOR_BLACK);
    LCD_DrawString(10, 120, "Test 3: 0123456789", COLOR_WHITE, COLOR_BLACK);
    
    uint16_t counter = 0;
    char counter_str[20];
    
    while (1) {
        // LED blink pattern depends on clock source
        if (clock_source == 0) {
            // HSE: fast double blink
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
            HAL_Delay(700);
        } else {
            // HSI: single slow blink
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
            HAL_Delay(500);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
            HAL_Delay(500);
        }
        
        // Update counter on display
        sprintf(counter_str, "Counter: %d", counter++);
        LCD_DrawString(10, 160, counter_str, COLOR_YELLOW, COLOR_BLACK);
        
        // Display current time (using HAL_GetTick)
        char time_str[30];
        sprintf(time_str, "Uptime: %lu ms", HAL_GetTick());
        LCD_DrawString(10, 180, time_str, COLOR_GREEN, COLOR_BLACK);
        
        HAL_Delay(1000);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Try HSE first (8 MHz external crystal on STM32F429I Discovery)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;   // HSE = 8 MHz, divide by 8 => 1 MHz
    RCC_OscInitStruct.PLL.PLLN = 336; // multiply by 336 => 336 MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // divide by 2 => 168 MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;   // 336 MHz / 7 = 48 MHz for USB
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // HSE failed - fall back to HSI
        clock_source = 1; // Mark as using HSI
        
        // Clear previous configuration
        RCC_OscInitStruct = (RCC_OscInitTypeDef){0};
        
        // Configure HSI with PLL for 168 MHz
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
        RCC_OscInitStruct.PLL.PLLM = 16;  // HSI = 16 MHz, divide by 16 => 1 MHz
        RCC_OscInitStruct.PLL.PLLN = 336; // multiply by 336 => 336 MHz
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // divide by 2 => 168 MHz
        RCC_OscInitStruct.PLL.PLLQ = 7;
        
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            Error_Handler();
        }
    }
    else
    {
        clock_source = 0; // Mark as using HSE
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_SPI5_Init(void)
{
    __HAL_RCC_SPI5_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    // Configure SPI5 pins (SCK, MOSI)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_9; // PF7=SCK, PF9=MOSI
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    // Configure CS and DC pins
    GPIO_InitStruct.Pin = LCD_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LCD_DC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_DC_GPIO_PORT, &GPIO_InitStruct);
    
    // Set CS and DC high initially
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    
    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 10;
    
    if (HAL_SPI_Init(&hspi5) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
}

void LCD_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_RESET); // Command
    HAL_SPI_Transmit(&hspi5, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void LCD_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET); // Data
    HAL_SPI_Transmit(&hspi5, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void LCD_Init(void)
{
    HAL_Delay(120);
    
    // Software reset
    LCD_WriteCommand(0x01);
    HAL_Delay(120);
    
    // Power control A
    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x34);
    LCD_WriteData(0x02);
    
    // Power control B
    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x30);
    
    // Driver timing control A
    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85);
    LCD_WriteData(0x00);
    LCD_WriteData(0x78);
    
    // Driver timing control B
    LCD_WriteCommand(0xEA);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    
    // Power on sequence control
    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);
    
    // Pump ratio control
    LCD_WriteCommand(0xF7);
    LCD_WriteData(0x20);
    
    // Power control 1
    LCD_WriteCommand(0xC0);
    LCD_WriteData(0x23);
    
    // Power control 2
    LCD_WriteCommand(0xC1);
    LCD_WriteData(0x10);
    
    // VCOM control 1
    LCD_WriteCommand(0xC5);
    LCD_WriteData(0x3E);
    LCD_WriteData(0x28);
    
    // VCOM control 2
    LCD_WriteCommand(0xC7);
    LCD_WriteData(0x86);
    
    // Memory access control
    LCD_WriteCommand(0x36);
    LCD_WriteData(0x48);
    
    // Pixel format
    LCD_WriteCommand(0x3A);
    LCD_WriteData(0x55);
    
    // Frame ratio control
    LCD_WriteCommand(0xB1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x18);
    
    // Display function control
    LCD_WriteCommand(0xB6);
    LCD_WriteData(0x08);
    LCD_WriteData(0x82);
    LCD_WriteData(0x27);
    
    // 3Gamma function disable
    LCD_WriteCommand(0xF2);
    LCD_WriteData(0x00);
    
    // Gamma curve selected
    LCD_WriteCommand(0x26);
    LCD_WriteData(0x01);
    
    // Positive gamma correction
    LCD_WriteCommand(0xE0);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x31);
    LCD_WriteData(0x2B);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x08);
    LCD_WriteData(0x4E);
    LCD_WriteData(0xF1);
    LCD_WriteData(0x37);
    LCD_WriteData(0x07);
    LCD_WriteData(0x10);
    LCD_WriteData(0x03);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x09);
    LCD_WriteData(0x00);
    
    // Negative gamma correction
    LCD_WriteCommand(0xE1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x14);
    LCD_WriteData(0x03);
    LCD_WriteData(0x11);
    LCD_WriteData(0x07);
    LCD_WriteData(0x31);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x48);
    LCD_WriteData(0x08);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x31);
    LCD_WriteData(0x36);
    LCD_WriteData(0x0F);
    
    // Sleep out
    LCD_WriteCommand(0x11);
    HAL_Delay(120);
    
    // Display on
    LCD_WriteCommand(0x29);
}

void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column address set
    LCD_WriteCommand(0x2A);
    LCD_WriteData(x0 >> 8);
    LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8);
    LCD_WriteData(x1 & 0xFF);
    
    // Page address set
    LCD_WriteCommand(0x2B);
    LCD_WriteData(y0 >> 8);
    LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8);
    LCD_WriteData(y1 & 0xFF);
    
    // Memory write
    LCD_WriteCommand(0x2C);
}

void LCD_Clear(uint16_t color)
{
    LCD_SetWindow(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    
    uint8_t data[2] = {color >> 8, color & 0xFF};
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    
    for(int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
    {
        HAL_SPI_Transmit(&hspi5, data, 2, HAL_MAX_DELAY);
    }
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    
    LCD_SetWindow(x, y, x, y);
    
    uint8_t data[2] = {color >> 8, color & 0xFF};
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    HAL_SPI_Transmit(&hspi5, data, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

// Simple 8x8 font for characters
const uint8_t font8x8[128][8] = {
    // Space (32)
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // A (65)
    [65] = {0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00},
    // B (66)
    [66] = {0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00},
    // C (67)
    [67] = {0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00},
    // D (68)
    [68] = {0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00},
    // E (69)
    [69] = {0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00},
    // Continue with more characters as needed...
    // H (72)
    [72] = {0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00},
    // I (73)
    [73] = {0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},
    // L (76)
    [76] = {0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00},
    // O (79)
    [79] = {0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00},
    // S (83)
    [83] = {0x78, 0xCC, 0x60, 0x30, 0x0C, 0xCC, 0x78, 0x00},
    // T (84)
    [84] = {0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},
    // Numbers 0-9
    [48] = {0x38, 0x6C, 0xC6, 0xD6, 0xC6, 0x6C, 0x38, 0x00}, // 0
    [49] = {0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00}, // 1
    [50] = {0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00}, // 2
    [51] = {0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00}, // 3
    [52] = {0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00}, // 4
    [53] = {0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00}, // 5
    [54] = {0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00}, // 6
    [55] = {0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00}, // 7
    [56] = {0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00}, // 8
    [57] = {0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00}, // 9
    // Some common characters
    [58] = {0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00}, // Colon (:)
};

void LCD_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color)
{
    if(ch < 32 || ch > 127) return; // Only printable ASCII
    
    for(int i = 0; i < 8; i++) {
        uint8_t line = font8x8[(int)ch][i];
        for(int j = 0; j < 8; j++) {
            if(line & (0x80 >> j)) {
                LCD_DrawPixel(x + j, y + i, color);
            } else {
                LCD_DrawPixel(x + j, y + i, bg_color);
            }
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bg_color)
{
    uint16_t start_x = x;
    
    while(*str) {
        if(*str == '\n') {
            y += 10;
            x = start_x;
        } else {
            LCD_DrawChar(x, y, *str, color, bg_color);
            x += 8;
            if(x >= LCD_WIDTH - 8) {
                x = start_x;
                y += 10;
            }
        }
        str++;
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
        for(volatile int i = 0; i < 500000; i++);
    }
}