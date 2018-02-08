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
#include <touchgfx/hal/GPIO.hpp>
#include "mbed.h"

using namespace touchgfx;

void GPIO::init()
{
}

DigitalOut led3(LED3);

void GPIO::set(GPIO_ID id)
{
//    gpioState[id] = 1;
//    if (id == VSYNC_FREQ)
//    {
//        led3 = 1;
//    }
//    else if (id == RENDER_TIME)
//    {
//        BSP_LED_Off(LED2);
//    }
//
//    else if (id == FRAME_RATE)
//    {
//        BSP_LED_Off(LED3);
//    }
//    else if (id == MCU_ACTIVE)
//    {
//        BSP_LED_Off(LED4);
//    }

}

void GPIO::clear(GPIO_ID id)
{
//    gpioState[id] = 0;
//    if (id == VSYNC_FREQ)
//    {
//        led3 = 0;
//    }
//    else if (id == RENDER_TIME)
//    {
//        BSP_LED_On(LED2);
//    }
//
//    else if (id == FRAME_RATE)
//    {
//        BSP_LED_On(LED3);
//    }
//    else if (id == MCU_ACTIVE)
//    {
//        BSP_LED_On(LED4);
//    }

}

void GPIO::toggle(GPIO_ID id)
{
//	if (id == VSYNC_FREQ){
//		led3 = led3 ^ 1;
//	}
//    if (get(id))
//    {
//        clear(id);
//    }
//    else
//    {
//        set(id);
//    }
}

bool GPIO::get(GPIO_ID id)
{
//    return gpioState[id];
	return 0;
}
