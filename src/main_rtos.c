#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "cmsis_os2.h" // CMSIS-RTOS2 API

// Standard library function implementations for embedded systems
#include <stdarg.h>
#include <stddef.h>

// Simple strlen implementation  
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

// Simple snprintf implementation 
int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    if (size == 0) return 0;
    
    // Simple implementation for the specific format used in your code
    if (format[0] == 'T' && format[1] == 'i' && format[2] == 'm' && format[3] == 'e') {
        // Format: "Time: %02d:%02d:%02d\r\n"
        int h = va_arg(args, int);
        int m = va_arg(args, int);
        int s = va_arg(args, int);
        
        str[0] = 'T'; str[1] = 'i'; str[2] = 'm'; str[3] = 'e'; str[4] = ':'; str[5] = ' ';
        str[6] = '0' + (h / 10); str[7] = '0' + (h % 10); str[8] = ':';
        str[9] = '0' + (m / 10); str[10] = '0' + (m % 10); str[11] = ':';
        str[12] = '0' + (s / 10); str[13] = '0' + (s % 10);
        str[14] = '\r'; str[15] = '\n'; str[16] = '\0';
        
        va_end(args);
        return 16;
    }
    
    va_end(args);
    return 0;
}

RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart2;
volatile uint8_t alarm_triggered = 0;

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // 1. Enable the External Oscillator (HSE)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;      // HSE = 8 MHz, divide by 8 => 1 MHz
    RCC_OscInitStruct.PLL.PLLN = 336;    // multiply by 336 => 336 MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // divide by 2 => 168 MHz system clock
    RCC_OscInitStruct.PLL.PLLQ = 7;      // for USB, SDIO, etc.
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // Initialization error handler
        while (1);
    }

    // 2. Set up the clocks for the buses
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 168 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;    // 42 MHz (max for APB1)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;    // 84 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        // Initialization error handler
        while (1);
    }

    // 3. Update SystemCoreClock variable
    SystemCoreClockUpdate();
}

void MX_RTC_Init(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);

void TimeDisplayTask(void *argument);
void AlarmHandlerTask(void *argument);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    // Send initial message to confirm UART is working
    char startup_msg[] = "RTC Alarm System Started!\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)startup_msg, strlen(startup_msg), 1000);
    
    MX_RTC_Init();

    osKernelInitialize();
    osThreadNew(TimeDisplayTask, NULL, NULL);
    osThreadNew(AlarmHandlerTask, NULL, NULL);
    osKernelStart();

    while (1) {
        // Execution will not reach here, tasks will manage
    }
}

void TimeDisplayTask(void *argument) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    char msg[64];
    for (;;) {
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // Required after GetTime()
        snprintf(msg, sizeof(msg), "Time: %02d:%02d:%02d\r\n",
                sTime.Hours, sTime.Minutes, sTime.Seconds);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
        osDelay(1000);
    }
}

void AlarmHandlerTask(void *argument) {
    for (;;) {
        if (alarm_triggered) {
            // Do something: blink LED, send UART, etc.
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Example - toggle an LED
            alarm_triggered = 0;
        }
        osDelay(100); // Polling delay
    }
}

void MX_RTC_Init(void)
{
    // Enable PWR and RTC clocks
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    
    // Configure LSE as RTC clock source
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // If LSE fails, use LSI as backup
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
        RCC_OscInitStruct.LSIState = RCC_LSI_ON;
        HAL_RCC_OscConfig(&RCC_OscInitStruct);
        __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
    } else {
        __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
    }
    
    __HAL_RCC_RTC_ENABLE();
    
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    
    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        Error_Handler();
    }

    // Set time if not set yet (on initial boot, or check backup reg)
    RTC_TimeTypeDef sTime = { .Hours = 16, .Minutes = 0, .Seconds = 0 };
    RTC_DateTypeDef sDate = { .Date = 1, .Month = RTC_MONTH_SEPTEMBER, .Year = 25 };
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Set up alarm (alarm at 16:00:10)
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.AlarmTime.Hours = 16;
    sAlarm.AlarmTime.Minutes = 0;
    sAlarm.AlarmTime.Seconds = 5;
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
    
    // Enable RTC Alarm IRQ in NVIC
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
    alarm_triggered = 1;
}

// Boilerplate
void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    
    // Configure PA5 for LED (User LED on Discovery board)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Turn off LED initially
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void MX_USART2_UART_Init(void) {
    // Enable clocks
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure GPIO pins for USART2 (PA2 = TX, PA3 = RX)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure UART
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

// Error handler function
void Error_Handler(void)
{
    // User can add their own implementation to report the HAL error return state
    __disable_irq();
    while (1)
    {
    }
}

// Add SystemClock_Config() appropriate for your board (from CubeMX or PlatformIO template)