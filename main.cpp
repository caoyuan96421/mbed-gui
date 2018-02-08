#include "mbed.h"
#include "rtos.h"
#include "stm32469i_discovery_sdram.h"
#include "stm32469i_discovery_qspi.h"
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <telescope/hardware/OmronE6CPDriver.h>
#include <telescope/config/TelescopeConfiguration.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dsi.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
Thread th;
Thread printer_th(osPriorityLow, 2048, NULL, "Printer Thread");

void blinker(DigitalOut *led) {
	while (1) {
		*led = !(*led);
		wait(0.1);
	}
}

void board_init(){
	BSP_SDRAM_Init();
	BSP_QSPI_Init();
	BSP_QSPI_EnableMemoryMappedMode();
}

Thread th_tgfx(osPriorityNormal, 16384, NULL, "GUI Thread");

void gui_thread(){
	touchgfx::HAL::getInstance()->taskEntry();
}

/* Mail */
typedef struct {
	char msg[64];
} mail_t;

typedef Mail<mail_t, 256> MB_t;
MB_t mbox;

Timer tim;
void printer(MB_t *mbox){
	while(true){
		mail_t *m = (mail_t *)mbox->get().value.p;
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
	len=sprintf(m->msg, "%6d>", tim.read_ms());
	vsprintf(&m->msg[len], format, argptr);
	mbox.put(m);

    va_end(  argptr);
}

TelescopeConfiguration config;
OmronE6CPDriver omron((DigitalOut*)config.getConfiguration("PECEncoderCLK"), (DigitalOut*)config.getConfiguration("PECEncoderLD"), (DigitalIn*)config.getConfiguration("PECEncoderDATA"));
Thread encoderThread;

void encoder_task(){
//	omron.init();
	while(true){
		uint32_t pos = omron.readPos();

		xprintf("Current encoder position: %d", pos);

		Thread::wait(10);
	}
}

// main() runs in its own thread in the OS
int main() {
	/*Power up wait*/

	Thread::wait(100);
	board_init();
	tim.start();

	/* Initialize touchGFX GUI Interface*/
	touchgfx::hw_init();
	touchgfx::touchgfx_init();



	th_tgfx.start(gui_thread);

	/* Start another blinker */
	th.start(callback(blinker, &led2));
	printer_th.start(callback(printer, &mbox));

	xprintf("System initialized");


	/* Start encoder readout */
	encoderThread.start(encoder_task);

	Thread::yield();
}

