/**
  ******************************************************************************
  * @file    stm32f4xx_hal_timebase_tim.c
  * @brief   HAL time base based on the hardware TIM6.
  *          
  *          This file overrides the default HAL timebase (SysTick) so that
  *          FreeRTOS can use SysTick exclusively for its scheduler.
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim6;

/**
  * @brief  This function configures the TIM6 as a time base source. 
  *         The time source is configured to have 1ms time base with a dedicated 
  *         Tick interrupt priority. 
  * @note   This function is called automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is reconfigured by HAL_RCC_ClockConfig().
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock = 0U;
  uint32_t              uwPrescalerValue = 0U;
  uint32_t              pFLatency;
  
  /* Enable TIM6 clock */
  __HAL_RCC_TIM6_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM6 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq();
  
  /* Compute the prescaler value to have TIM6 counter clock equal to 10 KHz (10000 Hz) */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 10000U) - 1U);
  
  /* Initialize TIM6 */
  htim6.Instance = TIM6;
  
  /* Initialize TIMx peripheral as follow:
       + Period = [(TIM6CLK/10000) - 1]. to have a (1/10000) s time base.
       + Prescaler = (uwTimclock/10000 - 1) to have a 10KHz counter clock.
       + ClockDivision = 0
       + Counter direction = Up
  */
  htim6.Init.Period = (10000U / 1000U) - 1U;  /* 1 ms */
  htim6.Init.Prescaler = uwPrescalerValue;
  htim6.Init.ClockDivision = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if (HAL_TIM_Base_Init(&htim6) == HAL_OK)
  {
    /* Start the TIM time Base generation in interrupt mode */
    if (HAL_TIM_Base_Start_IT(&htim6) == HAL_OK)
    {
      /* Configure the TIM6 IRQ priority */
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        /* Enable the TIM6 global Interrupt */
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        return HAL_ERROR;
      }
      HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
      
      /* Return function status */
      return HAL_OK;
    }
  }
  
  /* Return function status */
  return HAL_ERROR;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM6 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM6 update Interrupt */
  __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by enabling TIM6 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM6 Update interrupt */
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}

/**
  * @brief  This function handles TIM6 interrupt request.
  * @param  None
  * @retval None
  */
void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}
