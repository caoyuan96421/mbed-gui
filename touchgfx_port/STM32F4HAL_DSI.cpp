/******************************************************************************
 *
 * @brief     This file is part of the TouchGFX 4.8.0 evaluation distribution.
 *
 * @author    Draupner Graphics A/S <http://www.touchgfx.com>
 *
 ******************************************************************************
 *
 * @section Copyright
 *
 * Copyright (C) 2014-2016 Draupner Graphics A/S <http://www.touchgfx.com>.
 * All rights reserved.
 *
 * TouchGFX is protected by international copyright laws and the knowledge of
 * this source code may not be used to write a similar product. This file may
 * only be used in accordance with a license and should not be re-
 * distributed in any way without the prior permission of Draupner Graphics.
 *
 * This is licensed software for evaluation use, any use must strictly comply
 * with the evaluation license agreement provided with delivery of the
 * TouchGFX software.
 *
 * The evaluation license agreement can be seen on www.touchgfx.com
 *
 * @section Disclaimer
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Draupner Graphics A/S has
 * no obligation to support this software. Draupner Graphics A/S is providing
 * the software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Draupner Graphics A/S can not be held liable for any consequential,
 * incidental, or special damages, or any other relief, or for any claim by
 * any third party, arising from your use of this software.
 *
 *****************************************************************************/
#include <touchgfx/hal/OSWrappers.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <string.h>
#include <stdio.h>
#include <touchgfx_port/STM32F4DMA.hpp>
#include <touchgfx_port/STM32F4HAL_DSI.hpp>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dsi.h"
#include "stm32f4xx_hal_ltdc.h"
#include "stm32f4xx_hal_gpio.h"
#include "otm8009a.h"
#include "mbed.h"
#include "rtos.h"

/**
 * About this implementation:
 * This class is for use ONLY with the DSI peripheral. If you have a regular RGB interface display, use the STM32F4HAL.cpp class instead.
 *
 * This implementation assumes that the DSI is configured to be in adapted command mode, with tearing effect set to external pin.
 * Display will only be updated when there has actually been changes to the frame buffer.
 */

extern DSI_HandleTypeDef hdsi_eval;

//extern void xprintf(const char *, ...);

/* Request tear interrupt at specific scanline. Implemented in BoardConfiguration.cpp */
extern void LCD_ReqTear();

volatile bool displayRefreshing = false;
volatile bool refreshRequested = true;
static bool doubleBufferingEnabled = false;
static uint16_t* currFbBase = 0;
static uint16_t bitDepth = 0;
static const uint16_t gesture_thhd = 3;
static bool firstframe = true;

STM32F4HAL_DSI::STM32F4HAL_DSI(touchgfx::DMA_Interface& dma,
		touchgfx::LCD& display, touchgfx::TouchController& tc, uint16_t width,
		uint16_t height) :
		touchgfx::HAL(dma, display, tc, width, height)
{
	gestures.setDragThreshold(gesture_thhd);
	firstframe = true;
}

uint16_t* STM32F4HAL_DSI::getTFTFrameBuffer() const
{
	return currFbBase;
}

void STM32F4HAL_DSI::setFrameBufferStartAddress(void* adr, uint16_t depth,
		bool useDoubleBuffering, bool useAnimationStorage)
{
	// Make note of whether we are using double buffering.
	doubleBufferingEnabled = useDoubleBuffering;
	currFbBase = (uint16_t*) adr;
	bitDepth = depth;
	HAL::setFrameBufferStartAddress(adr, depth, useDoubleBuffering,
			useAnimationStorage);
}

void STM32F4HAL_DSI::setTFTFrameBuffer(uint16_t* adr)
{
	if (doubleBufferingEnabled)
	{
		__HAL_DSI_WRAPPER_DISABLE(&hdsi_eval);
		LTDC_Layer1->CFBAR = (uint32_t) adr;
		LTDC->SRCR |= LTDC_SRCR_IMR;
		currFbBase = adr;
		__HAL_DSI_WRAPPER_ENABLE(&hdsi_eval);
	}
}

void STM32F4HAL_DSI::configureInterrupts()
{
	// These two priorities MUST be EQUAL, and MUST be functionally lower than RTOS scheduler interrupts.
	NVIC_SetPriority(DMA2D_IRQn, 7);
	NVIC_SetPriority(DSI_IRQn, 7);
}

/* Enable LCD line interrupt, when entering video (active) area */
void STM32F4HAL_DSI::enableLCDControllerInterrupt()
{
	LCD_ReqTear();

	__HAL_DSI_CLEAR_FLAG(&hdsi_eval, DSI_IT_ER);
	__HAL_DSI_CLEAR_FLAG(&hdsi_eval, DSI_IT_TE);
	__HAL_DSI_ENABLE_IT(&hdsi_eval, DSI_IT_TE);
	__HAL_DSI_ENABLE_IT(&hdsi_eval, DSI_IT_ER);

}

void STM32F4HAL_DSI::disableInterrupts()
{
	NVIC_DisableIRQ(DMA2D_IRQn);
	NVIC_DisableIRQ(DSI_IRQn);
}

void STM32F4HAL_DSI::enableInterrupts()
{
	NVIC_EnableIRQ(DMA2D_IRQn);
	NVIC_EnableIRQ(DSI_IRQn);
}

uint64_t t = 0;
void STM32F4HAL_DSI::tick()
{
	HAL::tick();
	t++;
}

bool STM32F4HAL_DSI::beginFrame()
{
	core_util_critical_section_enter();
	refreshRequested = false;
	core_util_critical_section_exit();
	return HAL::beginFrame();
}


void STM32F4HAL_DSI::endFrame()
{
	HAL::endFrame();
	core_util_critical_section_enter();
	if (frameBufferUpdatedThisFrame)
	{
		refreshRequested = true;
	}
	core_util_critical_section_exit();
	if (firstframe)
	{
		HAL_DSI_ShortWrite(&hdsi_eval, 0, DSI_DCS_SHORT_PKT_WRITE_P0,
		OTM8009A_CMD_DISPON, 0);
		firstframe = false;
	}
}

extern "C" void DSI_IRQHandler(void)
{
	if (__HAL_DSI_GET_IT_SOURCE(&hdsi_eval,
			DSI_IT_TE) && __HAL_DSI_GET_FLAG(&hdsi_eval, DSI_FLAG_TE))
	{
		// Tearing effect interrupt. Occurs periodically (every 15.7 ms on 469 eval/disco boards)

		__HAL_DSI_CLEAR_FLAG(&hdsi_eval, DSI_IT_TE);
		GPIO::set(GPIO::VSYNC_FREQ);
		HAL::getInstance()->vSync();
		OSWrappers::signalVSync();
		if (/*!doubleBufferingEnabled &&*/ HAL::getInstance())
		{
			// In single buffering, only require that the system waits for display update to be finished if we
			// actually intend to update the display in this frame.
			HAL::getInstance()->lockDMAToFrontPorch(refreshRequested);
		}

		if (refreshRequested && !displayRefreshing)
		{
			// We have an update pending.
			if (doubleBufferingEnabled && HAL::getInstance())
			{
				// Swap frame buffers immediately instead of waiting for the task to be scheduled in.
				// Note: task will also swap when it wakes up, but that operation is guarded and will not have
				// any effect if already swapped.
				HAL::getInstance()->swapFrameBuffers();
			}

			HAL_DSI_Refresh(&hdsi_eval);
			displayRefreshing = true;
		}
		else
		{
			GPIO::clear(GPIO::VSYNC_FREQ);
		}
	}
	if (__HAL_DSI_GET_IT_SOURCE(&hdsi_eval,
			DSI_IT_ER) && __HAL_DSI_GET_FLAG(&hdsi_eval, DSI_FLAG_ER))
	{
		// End-of-refresh interrupt
		__HAL_DSI_CLEAR_FLAG(&hdsi_eval, DSI_IT_ER);

		// We are done refreshing.
		GPIO::clear(GPIO::VSYNC_FREQ);

		displayRefreshing = false;
		if (HAL::getInstance())
		{
			// Signal to the framework that display update has finished.
			HAL::getInstance()->frontPorchEntered();
		}
	}
}

extern "C" void DMA2D_IRQHandler(void)
{
	if (DMA2D->ISR & 2)
	{
		DMA2D->IFCR = 2;
		touchgfx::HAL::getInstance()->signalDMAInterrupt();
	}
}
