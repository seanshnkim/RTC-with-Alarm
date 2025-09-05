#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "main.h"

// Standard library function implementations for embedded systems
#include <stdarg.h>
#include <stddef.h>

// Simple strlen implementation  
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

// Simple number to string conversion
void uint_to_string(uint32_t num, char *str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    char temp[12];
    int i = 0;
    
    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Reverse the string
    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

// Simple time formatting
void format_time(char *buffer, int h, int m, int s) {
    buffer[0] = 'T'; buffer[1] = 'i'; buffer[2] = 'm'; buffer[3] = 'e'; 
    buffer[4] = ':'; buffer[5] = ' ';
    buffer[6] = '0' + (h / 10); buffer[7] = '0' + (h % 10); buffer[8] = ':';
    buffer[9] = '0' + (m / 10); buffer[10] = '0' + (m % 10); buffer[11] = ':';
    buffer[12] = '0' + (s / 10); buffer[13] = '0' + (s % 10);
    buffer[14] = '\r'; buffer[15] = '\n'; buffer[16] = '\0';
}

// Simple clock frequency formatting
void format_clock_msg(char *buffer, uint32_t freq) {
    char freq_str[12];
    uint_to_string(freq, freq_str);
    
    // Build "SysClock: XXXXXX Hz\r\n"
    char *msg = "SysClock: ";
    int i = 0;
    while (msg[i]) {
        buffer[i] = msg[i];
        i++;
    }
    
    int j = 0;
    while (freq_str[j]) {
        buffer[i++] = freq_str[j++];
    }
    
    buffer[i++] = ' '; buffer[i++] = 'H'; buffer[i++] = 'z';
    buffer[i++] = '\r'; buffer[i++] = '\n'; buffer[i] = '\0';
}

RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart1;  // Using UART1 instead of UART2
volatile uint8_t alarm_triggered = 0;
volatile uint32_t tick_count = 0;

void SystemClock_Config(void);
void MX_RTC_Init(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    // Check actual system clock frequency
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    char clock_msg[64];
    format_clock_msg(clock_msg, sysclk);
    HAL_UART_Transmit(&huart1, (uint8_t*)clock_msg, strlen(clock_msg), 1000);
    
    // Send initial message to confirm UART is working
    char startup_msg[] = "=== RTC Alarm System Started! ===\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)startup_msg, strlen(startup_msg), 1000);
    
    MX_RTC_Init();
    
    // Send RTC init confirmation
    char rtc_msg[] = "RTC Initialized!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)rtc_msg, strlen(rtc_msg), 1000);

    // Main loop without RTOS
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    char msg[64];
    uint32_t led_counter = 0;  // Counter for LED blinking
    
    while (1) {
        // Blink LED every 2 seconds to show program is running
        led_counter++;
        if (led_counter >= 2) {
            HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
            led_counter = 0;
        }
        
        // Display time every second
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // Required after GetTime()
        format_time(msg, sTime.Hours, sTime.Minutes, sTime.Seconds);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
        
        // Check alarm
        if (alarm_triggered) {
            char alarm_msg[] = "*** ALARM TRIGGERED! ***\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)alarm_msg, strlen(alarm_msg), 1000);
            
            // Blink LED rapidly 5 times when alarm triggers
            for(int i = 0; i < 10; i++) {
                HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
                HAL_Delay(100);
            }
            
            alarm_triggered = 0;
        }
        
        HAL_Delay(1000); // 1 second delay
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Initializes the RCC Oscillators according to the specified parameters
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    // if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    // {
    //     // HSE failed, let's try HSI instead
    //     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    //     RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    //     RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    //     RCC_OscInitStruct.PLL.PLLM = 16;  // HSI = 16 MHz, divide by 16 => 1 MHz
    //     if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    //     {
    //         Error_Handler();
    //     }
    // }
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Initializes the CPU, AHB and APB buses clocks
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

void MX_USART1_UART_Init(void)
{
    // Enable clocks
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure GPIO pins for USART1 (PA9 = TX, PA10 = RX) - correct for Discovery board
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure UART
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_GPIO_Init(void)
{
    // Enable clocks
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure PG13 for Green LED on Discovery board
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
    // Turn off LED initially
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
}

void MX_RTC_Init(void)
{
    // Enable PWR and RTC clocks
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    
    // Use LSI as RTC clock source (more reliable for testing)
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
    
    __HAL_RCC_RTC_ENABLE();
    
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;  // For LSI (~32kHz)
    hrtc.Init.SynchPrediv = 249;   // Adjusted for LSI
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        // Send error message if RTC init fails
        char error_msg[] = "RTC Init Failed!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)error_msg, strlen(error_msg), 1000);
        Error_Handler();
    }
    
    // Send success message
    char rtc_init_msg[] = "RTC Init Success!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)rtc_init_msg, strlen(rtc_init_msg), 1000);

    // Set initial time
    RTC_TimeTypeDef sTime = { .Hours = 10, .Minutes = 0, .Seconds = 0 };
    RTC_DateTypeDef sDate = { .Date = 5, .Month = RTC_MONTH_SEPTEMBER, .Year = 25 };
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Set up alarm (alarm at 10:00:03 - 3 seconds after startup)
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.AlarmTime.Hours = 10;
    sAlarm.AlarmTime.Minutes = 0;
    sAlarm.AlarmTime.Seconds = 3;
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    
    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        char alarm_error_msg[] = "Alarm Setup Failed!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)alarm_error_msg, strlen(alarm_error_msg), 1000);
        Error_Handler();
    }
    
    // Send alarm setup success message
    char alarm_ok_msg[] = "Alarm Set for 10:00:03!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)alarm_ok_msg, strlen(alarm_ok_msg), 1000);
    
    // Enable RTC Alarm IRQ in NVIC
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    alarm_triggered = 1;
}

// Error handler function
void Error_Handler(void)
{
    // Disable interrupts and stay in infinite loop
    __disable_irq();
    while (1)
    {
        // Blink LED rapidly to indicate error
        HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
        for(volatile int i = 0; i < 100000; i++);
    }
}
