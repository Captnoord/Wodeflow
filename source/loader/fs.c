#include <ogcsys.h>
#include <locale.h>
#include <fat.h>
#include <ogc/usbstorage.h>

#include "sdhc.h"
#include "fs.h"
#include "gecko.h"

/* Disc interfaces */
extern const DISC_INTERFACE __io_wiisd;
extern const DISC_INTERFACE __io_sdhc;
extern DISC_INTERFACE __io_usbstorage;

// read-only
extern const DISC_INTERFACE __io_sdhc_ro;

bool g_sdOK = false;
bool g_usbOK = false;

#define CACHE   4
#define SECTORS 32

bool Fat_SDAvailable(void)
{
	gprintf("SD Available: %d\n", g_sdOK);
	return g_sdOK;
}

bool Fat_USBAvailable(void)
{
	gprintf("USB Available: %d\n", g_usbOK);
	return g_usbOK;
}

void Fat_Unmount(void)
{
	if (g_sdOK)
	{
		fatUnmount("sd:");
		__io_wiisd.shutdown();
		__io_sdhc.shutdown();	
		g_sdOK = false;
	}
	if (g_usbOK)
	{
		fatUnmount("usb:");
		g_usbOK = false;
		__io_usbstorage.shutdown();
	}
}

bool Fat_Mount(void)
{
	Fat_Unmount();
	if (!g_sdOK)
	{
		__io_wiisd.startup();
		g_sdOK = fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS);
	}
	if (!g_sdOK)
	{
		__io_sdhc.startup();
		g_sdOK = fatMount("sd", &__io_sdhc, 0, CACHE, SDHC_SECTOR_SIZE);
	}
	if (!g_usbOK)
	{
		__io_usbstorage.startup();
		g_usbOK = fatMount("usb", &__io_usbstorage, 0, CACHE, SECTORS);
	}
	return g_sdOK;
}
