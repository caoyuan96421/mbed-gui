#include "mbed.h"
#include "rtos.h"
#include "stm32469i_discovery_sdram.h"
#include "stm32469i_discovery_qspi.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include "usbd_desc.h"
#include "usbd_msc_storage.h"
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dsi.h"
#include "StarCatalog.h"

DigitalOut led(LED1);
Thread blinker_thread;
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

Thread th_tgfx(osPriorityNormal1, 16384, NULL, "GUI Thread");

void gui_thread()
{
	touchgfx::HAL::getInstance()->taskEntry();
}

/* Mail */
typedef struct
{
	char msg[254];
	short len;
} mail_t;

typedef Mail<mail_t, 256> MB_t;

MB_t mbox;

UARTSerial serial(USBTX, USBRX, 115200);

Timer tim;
void printer(MB_t *mbox)
{
	while (true)
	{
		osEvent evt = mbox->get();
		if (evt.status == osEventMail)
		{
			mail_t *m = (mail_t *) evt.value.p;
			serial.write(m->msg, m->len);
			mbox->free(m);
			serial.write("\r\n", 2);
		}
		else
		{
			printf("Something wrong.\n");
		}
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

	mail_t *m;

	if (IS_IRQ_MODE() || IS_IRQ_MASKED())
	{
		m = mbox.alloc(0);
		if (!m)
			return;
	}
	else
	{
		m = mbox.alloc(osWaitForever);
		if (!m)
			return;
	}
	len = snprintf(m->msg, sizeof(m->msg), "%6d>", tim.read_ms());
	len += vsnprintf(&m->msg[len], sizeof(m->msg) - len, format, argptr);
	m->len = len;
	mbox.put(m);

	va_end(argptr);
}

void stprintf(FileHandle &f, const char *fmt, ...)
{
	char buf[512];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	f.write(buf, len);
}

#include "SDIOBlockDevice.h"
#include "FATFileSystem.h"
/**
 * SD card reader hardware configuration
 */
SDIOBlockDevice &sd = SDIOBlockDevice::getInstance();
FATFileSystem fs("sdcard");

USBD_HandleTypeDef hUSBDDevice;
bool mounted = false;

// main() runs in its own thread in the OS
int main()
{
	/*Power up wait*/

	Thread::wait(100);
	board_init();
	tim.start();


	blinker_thread.start(blinker);
	printer_th.start(callback(printer, &mbox));

	xprintf("System initialized");

	printf("Mounting SD card...\n");

//	if (FATFileSystem::format(&sd, 512) == 0)
//	{
//		printf("Card formatted.\n");
//	}
//	else
//	{
//		printf("Card format error.\n");
//	}

	if (fs.mount(&sd) != 0)
	{
		debug("Error: failed to mount SD card.\n");
	}
	else
	{
		mounted = true;
		FILE *fp = fopen("/sdcard/hello.txt", "r");

		if (fp == NULL)
		{
			debug("Error: file not found.\n");
		}
		else
		{
			char buf[64];
			fgets(buf, sizeof(buf), fp);
			printf("%s\n", buf);
			fclose(fp);
		}
	}

	// Start USB

	if (mounted)
	{
		USBD_Init(&hUSBDDevice, &USB_Desc, 0);
		USBD_RegisterClass(&hUSBDDevice, USBD_MSC_CLASS);
		USBD_MSC_RegisterStorage(&hUSBDDevice, &USBD_MSC_fops);
		USBD_Start(&hUSBDDevice);
	}

	StarCatalog::getInstance(); // Initialize Catalog

	/* Initialize touchGFX GUI Interface*/
	touchgfx::hw_init();
	touchgfx::touchgfx_init();
	th_tgfx.start(gui_thread);

//	PwmOut out(D3);
//	InterruptIn in(D7);
//	in.fall(cb);
//
//	Thread::wait(2000);
//	out.period_us(2);
//	out.write(0.5);


	while (1)
	{
		Thread::wait(1000);
//		xprintf("");
//		time_t t = time(NULL);
//
//		xprintf("Time as seconds since January 1, 1970 = %d\n", t);
//
//		xprintf("Time as a basic string = %s", ctime(&t));
//
//		char buffer[32];
//		strftime(buffer, 32, "%I:%M %p\n", localtime(&t));
//		xprintf("Time as a custom formatted string = %s", buffer);
	}
}

