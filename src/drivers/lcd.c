/**
 * @file lcd.c
 * @brief LCD Display Driver Implementation for STM32F429I Discovery Board
 * @author Generated for STM32F429I Discovery LCD
 * @date 2025
 */

#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"

// Simple 8x8 font for basic characters
const uint8_t font8x8_basic[128][8] = {
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    [48] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, // 0
    [49] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 1
    [50] = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00}, // 2
    [51] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 3
    [52] = {0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C, 0x00}, // 4
    [53] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 5
    [54] = {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 6
    [55] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00}, // 7
    [56] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 8
    [57] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00}, // 9
    [65] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, // A
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // B
    [67] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
    [68] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
    [69] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00}, // E
    [70] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00}, // F
    [71] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}, // G
    [72] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // H
    [73] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
    [74] = {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00}, // J
    [75] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
    [76] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // L
    [77] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // M
    [78] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // N
    [79] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
    [80] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // P
    [81] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x0E, 0x00}, // Q
    [82] = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00}, // R
    [83] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // S
    [84] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
    [85] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // U
    [86] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // V
    [87] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W
    [88] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // X
    [89] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // Y
    [90] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // Z
    [97] = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, // a
    [98] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00}, // b
    [99] = {0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00}, // c
    [100] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00}, // d
    [101] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // e
    [102] = {0x0E, 0x18, 0x18, 0x3E, 0x18, 0x18, 0x18, 0x00}, // f
    [103] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C}, // g
    [104] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // h
    [105] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00}, // i
    [106] = {0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C}, // j
    [107] = {0x60, 0x60, 0x6C, 0x78, 0x6C, 0x66, 0x66, 0x00}, // k
    [108] = {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // l
    [109] = {0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // m
    [110] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // n
    [111] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // o
    [112] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60}, // p
    [113] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06}, // q
    [114] = {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00}, // r
    [115] = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00}, // s
    [116] = {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00}, // t
    [117] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, // u
    [118] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // v
    [119] = {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00}, // w
    [120] = {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00}, // x
    [121] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x0C, 0x78}, // y
    [122] = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}, // z
};

/**
 * @brief Initialize LCD GPIO pins for manual SPI control
 */
void MX_LCD_GPIO_Init(void)
{
    // Enable GPIO clocks for all LCD pins used on STM32F429I-Discovery
    __HAL_RCC_GPIOF_CLK_ENABLE(); // PF7 (SCK), PF9 (MOSI), PF10 (RST)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // PC2 (CS)
    __HAL_RCC_GPIOD_CLK_ENABLE(); // PD13 (DC)

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure SCK pin (PF7)
    GPIO_InitStruct.Pin = LCD_SCK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_SCK_GPIO_PORT, &GPIO_InitStruct);

    // Configure MOSI pin (PF9)
    GPIO_InitStruct.Pin = LCD_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_MOSI_GPIO_PORT, &GPIO_InitStruct);

    // Configure CS pin (PC2)
    GPIO_InitStruct.Pin = LCD_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStruct);

    // Configure DC pin (PD13)
    GPIO_InitStruct.Pin = LCD_DC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_DC_GPIO_PORT, &GPIO_InitStruct);

    // Configure RST pin (PF10)
    GPIO_InitStruct.Pin = LCD_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_RST_GPIO_PORT, &GPIO_InitStruct);

    // Set initial pin states
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);        // CS high (inactive)
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);        // DC high
    HAL_GPIO_WritePin(LCD_SCK_GPIO_PORT, LCD_SCK_PIN, GPIO_PIN_RESET);    // SCK low
    HAL_GPIO_WritePin(LCD_MOSI_GPIO_PORT, LCD_MOSI_PIN, GPIO_PIN_RESET);  // MOSI low
    HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_PIN, GPIO_PIN_SET);      // RST high
}

/**
 * @brief Manual SPI bit-banging transmission
 * @param data Byte to transmit
 */
void Manual_SPI_Transmit(uint8_t data)
{
    for(int i = 7; i >= 0; i--) {
        // Set MOSI pin based on current bit (MSB first)
        if(data & (1 << i)) {
            HAL_GPIO_WritePin(LCD_MOSI_GPIO_PORT, LCD_MOSI_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LCD_MOSI_GPIO_PORT, LCD_MOSI_PIN, GPIO_PIN_RESET);
        }
        
        // Clock high
        HAL_GPIO_WritePin(LCD_SCK_GPIO_PORT, LCD_SCK_PIN, GPIO_PIN_SET);
        // Small delay for setup time
        for(volatile int j = 0; j < 10; j++);
        
        // Clock low
        HAL_GPIO_WritePin(LCD_SCK_GPIO_PORT, LCD_SCK_PIN, GPIO_PIN_RESET);
        // Small delay for hold time
        for(volatile int j = 0; j < 10; j++);
    }
}

/**
 * @brief Send command to LCD
 * @param cmd Command byte to send
 */
void LCD_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);   // CS active (low)
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_RESET);   // DC low for command
    Manual_SPI_Transmit(cmd);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);     // CS inactive (high)
}

/**
 * @brief Send data to LCD
 * @param data Data byte to send
 */
void LCD_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);   // CS active (low)
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);     // DC high for data
    Manual_SPI_Transmit(data);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);     // CS inactive (high)
}

/**
 * @brief Initialize LCD display
 */
void LCD_Init(void)
{
    // Hardware reset sequence
    HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);

    // Software reset (optional but recommended)
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

/**
 * @brief Set drawing window on LCD
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
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

/**
 * @brief Clear entire LCD with specified color
 * @param color RGB565 color value
 */
void LCD_Clear(uint16_t color)
{
    LCD_SetWindow(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);   // CS active
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);     // DC high for data
    
    for(int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
    {
        Manual_SPI_Transmit(color_high);
        Manual_SPI_Transmit(color_low);
    }
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);     // CS inactive
}

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color value
 */
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    
    LCD_SetWindow(x, y, x, y);
    
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);   // CS active
    HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET);     // DC high for data
    Manual_SPI_Transmit(color_high);
    Manual_SPI_Transmit(color_low);
    HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);     // CS inactive
}

/**
 * @brief Fill a rectangular area with specified color
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param color RGB565 color value
 */

/**
 * @brief Draw a single character
 * @param x X coordinate
 * @param y Y coordinate
 * @param ch Character to draw
 * @param color Foreground color (RGB565)
 * @param bg_color Background color (RGB565)
 */
void LCD_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color)
{
    if(ch < 32 || ch > 127) return; // Only printable ASCII
    
    for(int i = 0; i < 8; i++) {
        uint8_t line = font8x8_basic[(int)ch][i];
        for(int j = 0; j < 8; j++) {
            if(line & (0x80 >> j)) {
                LCD_DrawPixel(x + j, y + i, color);
            } else {
                LCD_DrawPixel(x + j, y + i, bg_color);
            }
        }
    }
}

/**
 * @brief Draw a string
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Foreground color (RGB565)
 * @param bg_color Background color (RGB565)
 */
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

/**
 * @brief Print task message with line clearing
 * @param x X coordinate
 * @param y Y coordinate
 * @param message Message string
 * @param color Text color (RGB565)
 */
void LCD_PrintTask(uint16_t x, uint16_t y, char* message, uint16_t color)
{
    // Suspend all tasks to prevent LCD interference during update
    vTaskSuspendAll();
    
    // Clear the line first (draw black rectangle)
    for(int i = 0; i < 220; i++) {
        for(int j = 0; j < 10; j++) {
            LCD_DrawPixel(x + i, y + j, COLOR_BLACK);
        }
    }
    
    // Draw the new message
    LCD_DrawString(x, y, message, color, COLOR_BLACK);
    
    // Resume task scheduler
    xTaskResumeAll();
}