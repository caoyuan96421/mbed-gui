#include <driver/AMIS30543StepperDriver.h>
#include "mbed.h"
#include "rtos.h"
#include "stm32469i_discovery_sdram.h"
#include "stm32469i_discovery_qspi.h"
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dsi.h"
#include "StepOut.h"

DigitalOut led(LED1);
StepOut led2(PB_1);
Thread th;
Thread printer_th(osPriorityNormal, 2048, NULL, "Printer Thread");

void blinker()
{
	while (1)
	{
		led = !led;
		wait(0.2);
	}
}

void board_init()
{
	BSP_SDRAM_Init();
	BSP_QSPI_Init();
	BSP_QSPI_EnableMemoryMappedMode();
}

Thread th_tgfx(osPriorityNormal, 16384, NULL, "GUI Thread");

void gui_thread()
{
	touchgfx::HAL::getInstance()->taskEntry();
}

/* Mail */
typedef struct
{
	char msg[128];
} mail_t;

typedef Mail<mail_t, 256> MB_t;
MB_t mbox;

Timer tim;
void printer(MB_t *mbox)
{
	while (true)
	{
		mail_t *m = (mail_t *) mbox->get().value.p;
		printf("%s\r\n", m->msg);
		mbox->free(m);
	}
}

/**
 * Printf for debugging use. Takes about 20us for each call. Can be called from any context
 */
void xprintf(const char* format, ...)
{
	uint16_t len;
	va_list argptr;
	va_start(argptr, format);

	mail_t *m = mbox.alloc();
	len = sprintf(m->msg, "%6d>", tim.read_ms());
	len += vsprintf(&m->msg[len], format, argptr);
	mbox.put(m);

	va_end(argptr);
}

extern void test_stepper();

// main() runs in its own thread in the OS
int main()
{
	/*Power up wait*/

	Thread::wait(100);
	board_init();
	tim.start();

	/* Initialize touchGFX GUI Interface*/
	touchgfx::hw_init();
	touchgfx::touchgfx_init();

	th_tgfx.start(gui_thread);

	th.start(blinker);
	printer_th.start(callback(printer, &mbox));

	xprintf("System initialized");

	led2.setPeriod(1);
	led2.start();

	test_stepper();

	Thread::yield();
}

