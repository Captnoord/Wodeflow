#include <stdio.h>
#include <ogcsys.h>
#include <string.h>

#include "apploader.h"
#include "patchcode.h"
#include "disc.h"
#include "wip.h"
#include "wdvd.h"
#include "videopatch.h"
#include "gecko.h"

typedef struct _SPatchCfg
{
	bool cheat;
	u8 vidMode;
	GXRModeObj *vmode;
	bool vipatch;
	bool countryString;
	u8 patchVidModes;
} SPatchCfg;

/* Apploader function pointers */
typedef int   (*app_main)(void **dst, int *size, int *offset);
typedef void  (*app_init)(void (*report)(const char *fmt, ...));
typedef void *(*app_final)();
typedef void  (*app_entry)(void (**init)(void (*report)(const char *fmt, ...)), int (**main)(), void *(**final)());

/* Apploader pointers */
static u8 *appldr = (u8 *)0x81200000;


/* Constants */
#define APPLDR_OFFSET	0x2440

/* Variables */
static u32 buffer[0x20] ATTRIBUTE_ALIGN(32);

static void maindolpatches(void *dst, int len, bool cheat, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes);

static void __noprint(const char *fmt, ...)
{
}


s32 Apploader_Run(entry_point *entry, bool cheat, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes)
{
	void *dst = NULL;
	int len = 0;
	int offset = 0;
	app_entry appldr_entry;
	app_init  appldr_init;
	app_main  appldr_main;
	app_final appldr_final;

	u32 appldr_len;
	s32 ret;

	wipreset();

	SYS_SetArena1Hi((void *)0x816FFFF0);
	/* Read apploader header */
	ret = WDVD_Read(buffer, 0x20, APPLDR_OFFSET);
	if (ret < 0) {
		gprintf("WDVD_Read failed (for APPLDR_OFFSET): %d\n", ret);
		return ret;
	}
	ghexdump(buffer, 0x20);

	/* Calculate apploader length */
	appldr_len = buffer[5] + buffer[6];

	/* Read apploader code */
	// Either you limit memory usage or you don't touch the heap after that, because this is writing at 0x1200000
	ret = WDVD_Read(appldr, appldr_len, APPLDR_OFFSET + 0x20);
	if (ret < 0) {
		gprintf("Reading apploader failed: %d\n", ret);
		return ret;
	}
	DCFlushRange(appldr, appldr_len);

	/* Set apploader entry function */
	appldr_entry = (app_entry)buffer[4];

	/* Call apploader entry */
	appldr_entry(&appldr_init, &appldr_main, &appldr_final);

	/* Initialize apploader */
	appldr_init(__noprint);

	while (appldr_main(&dst, &len, &offset))
	{
		/* Read data from DVD */
		WDVD_Read(dst, len, (u64)(offset << 2));
		maindolpatches(dst, len, cheat, vidMode, vmode, vipatch, countryString, patchVidModes);
	}
	WDVD_Close();

	gprintf("Applying wip patches...");
	
	do_wip_patches();
	wipreset();

	gprintf("done\n");
	
	/* Set entry point from apploader */
	*entry = appldr_final();

	DCFlushRange((void*)0x80000000, 0x3f00);

	return 0;
}

static void PatchCountryStrings(void *Address, int Size)
{
	u8 SearchPattern[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 PatchData[4] = { 0x00, 0x00, 0x00, 0x00 };
	u8 *Addr = (u8*)Address;
	int wiiregion = CONF_GetRegion();

	switch (wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 'J';
			SearchPattern[2] = 'P';
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 'E';
			SearchPattern[2] = 'U';
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 'K';
			SearchPattern[2] = 'R';
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 'C';
			SearchPattern[2] = 'N';
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 'U';
			SearchPattern[2] = 'S';
	}
	switch (((const u8 *)0x80000000)[3])
	{
		case 'J':
			PatchData[1] = 'J';
			PatchData[2] = 'P';
			break;
		case 'D':
		case 'F':
		case 'P':
		case 'X':
		case 'Y':
			PatchData[1] = 'E';
			PatchData[2] = 'U';
			break;

		case 'E':
		default:
			PatchData[1] = 'U';
			PatchData[2] = 'S';
	}
	while (Size >= 4)
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
		{
			//*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			Addr += 1;
			//*Addr = PatchData[3];
			Addr += 1;
			Size -= 4;
		}
		else
		{
			Addr += 4;
			Size -= 4;
		}
}

static void maindolpatches(void *dst, int len, bool cheat, u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes)
{
	DCFlushRange(dst, len);
	
	gprintf("Applying patches...");
	wipregisteroffset((u32) dst, len);
	
	patchVideoModes(dst, len, vidMode, vmode, patchVidModes);

	if (cheat)
		dogamehooks(dst, len);
	if (vipatch)
		vidolpatcher(dst, len);
	if (configbytes[0] != 0xCD)
		langpatcher(dst, len);
	if (countryString) // Country Patch by WiiPower
		PatchCountryStrings(dst, len);
	
	gprintf("done\n");
	DCFlushRange(dst, len);
}
