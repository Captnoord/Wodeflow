#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <di/di.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>

#include "apploader.h"
#include "disc.h"
#include "wdvd.h"
#include "sys.h"
#include "videopatch.h"
#include "patchcode.h"
#include "fst.h"
#include "wode.h"

#include "gecko.h"

#define ALIGNED(x) __attribute__((aligned(x)))

/* Constants */
#define PTABLE_OFFSET	0x40000

//appentrypoint 
u32 appentrypoint;
	
/* Disc pointers */
static u32 buffer[0x20] ALIGNED(32);
static u8  *diskid = (u8  *)0x80000000;

GXRModeObj *vmode = NULL;
u32 vmode_reg = 0;

extern void __exception_closeall();

static u8	Tmd_Buffer[0x49e4 + 0x1C] ALIGNED(32);

#define        Sys_Magic		((u32*)0x80000020)
#define        Version			((u32*)0x80000024)
#define        Arena_L			((u32*)0x80000030)
#define        BI2				((u32*)0x800000f4)
#define        Bus_Speed		((u32*)0x800000f8)
#define        CPU_Speed		((u32*)0x800000fc)

void __Disc_SetLowMem(void)
{
	// Patch in info missing from apploader reads
	*Sys_Magic	= 0x0d15ea5e;
	*Version	= 1;
	*Arena_L	= 0x00000000;
	*Bus_Speed	= 0x0E7BE2C0;
	*CPU_Speed	= 0x2B73A840;

	/* http://www.wiibrew.org/wiki/Memory_Map */
	/* Setup low memory */
	*(vu32 *)0x80000030 = 0x00000000;
	*(vu32 *)0x80000060 = 0x38A00040;
	*(vu32 *)0x800000E4 = 0x80431A80;
	*(vu32 *)0x800000EC = 0x81800000;
	*(vu32 *)0x800000F0 = 0x01800000;       // Simulated Memory Size
	*BI2 = 0x817E5480;
	*(vu32 *)0x800000F8 = 0x0E7BE2C0;
	*(vu32 *)0xCD00643C = 0x00000000;       // 32Mhz on Bus

	// From NeoGamme R4 (WiiPower)
	*(vu32 *)0x800030F0 = 0x0000001C;
	*(vu32 *)0x8000318C = 0x00000000;
	*(vu32 *)0x80003190 = 0x00000000;

	/* Copy disc ID (online check) */
	memcpy((void *)0x80003180, (void *)0x80000000, 4);

	/* Flush cache */
	DCFlushRange((void *)0x80000000, 0x3F00);
}

void __Disc_SelectVMode(u8 videoselected)
{
    vmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();

	/* Select video mode register */
	switch (CONF_GetVideo())
	{
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0)
			{
				vmode_reg = VI_EURGB60;
				vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			}
			else
				vmode_reg = VI_PAL;
			break;

		case CONF_VIDEO_MPAL:
			vmode_reg = VI_MPAL;
			break;

		case CONF_VIDEO_NTSC:
			vmode_reg = VI_NTSC;
			break;
	}

    switch (videoselected)
	{
		case 0: // DEFAULT (DISC/GAME)
			/* Select video mode */
			switch (diskid[3])
			{
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					if (CONF_GetVideo() != CONF_VIDEO_PAL)
					{
						vmode_reg = VI_PAL;
						vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					}
					break;
				// NTSC
				case 'E':
				case 'J':
				default:
					if (CONF_GetVideo() != CONF_VIDEO_NTSC)
					{
						vmode_reg = VI_NTSC;
						vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
			}
			break;
		case 1: // PAL50
			vmode =  &TVPal528IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 2: // PAL60
			vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			vmode_reg = progressive ? TVEurgb60Hz480Prog.viTVMode >> 2 : vmode->viTVMode >> 2;
			break;
		case 3: // NTSC
			vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 4: // AUTO PATCH TO SYSTEM
		case 5: // SYSTEM
			break;
		case 6: // PROGRESSIVE 480P(NTSC + PATCH ALL)
			vmode = &TVNtsc480Prog;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		default:
			break;
	}
}

void __Disc_SetVMode(void)
{
	/* Set video mode register */
	*(vu32 *)0x800000CC = vmode_reg;

	/* Set video mode */
	if (vmode != 0)
	{
		VIDEO_Configure(vmode);
	}
	
	/* Setup video  */
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

void __Disc_SetTime(void)
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 __Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0, table_offset = 0;
	u32 cnt, nb_partitions;
	s32 ret;
	
	/* Read partition info */
	ret = WDVD_UnencryptedRead(buffer, 0x20, PTABLE_OFFSET);
	if (ret < 0) {
		return ret;
	}

	/* Get data */
	nb_partitions = buffer[0];
	table_offset  = buffer[1] << 2;
	
	if (nb_partitions > 8) {
		return -1;
	}
	
	/* Read partition table */
	ret = WDVD_UnencryptedRead(buffer, 0x20, table_offset);
	if (ret < 0) {
		return ret;
	}
	
	/* Find game partition */
	for (cnt = 0; cnt < nb_partitions; cnt++) {
		u32 type = buffer[cnt * 2 + 1];

		/* Game partition */
		if(!type)
			offset = buffer[cnt * 2] << 2;
	}

	/* No game partition found */
	if (!offset) {
		gprintf("No game offset partition found\n");
		return -1;
	}

	/* Set output buffer */
	*outbuf = offset;

	WDVD_Seek(offset);

	return 0;
}

s32 Disc_Init(void)
{
	/* Init DVD subsystem */
	return WDVD_Init();
}

s32 Disc_Open(void)
{
	s32 ret;

	/* Reset drive */
	ret = WDVD_Reset();
	if (ret < 0)
		return ret;

	memset(diskid, 0, 32);

	/* Read disc ID */
	return WDVD_ReadDiskId(diskid);
}

s32 Disc_Wait(void)
{
	u32 cover = 0;
	s32 ret;

	/* Wait for disc */
	while (!(cover & 0x2)) {
		/* Get cover status */
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 Disc_ReadHeader(void *outbuf)
{
	/* Read disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct discHdr), 0);
}

s32 Disc_IsWii(void)
{
	struct discHdr *header = (struct discHdr *)buffer;

	s32 ret;

	/* Read disc header */
	ret = Disc_ReadHeader(header);
	if (ret < 0)
		return ret;

	/* Check magic word */
	if (header->magic != WII_MAGIC)
		return -1;

	return 0;
}


s32 Disc_BootPartition(u64 offset, u8 vidMode, const u8 *cheat, u32 cheatSize, bool vipatch, bool countryString, u8 patchVidMode)
{
	entry_point p_entry;

	gprintf("Open partition at offset: 0x%08x\n", offset);
	s32 ret = WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer);
	if (ret < 0) {
		gprintf("Open partition failed: %d\n", ret);
		return ret;
	}
		
	/* Disconnect Wiimote */
    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

	/* Reload IOS into the correct IOS */
	u8 ios = Tmd_Buffer[0x18B];
	
	gprintf("Game requires ios %d\n", ios);
	if (ios != IOS_GetVersion()) {
		WDVD_ClosePartition();
		WDVD_Close();
	
		gprintf("Reloading IOS...");
		ret = IOS_ReloadIOS(ios);
		if (ret < 0) {
			gprintf("failed: %d\n", ret);
		} else {
			gprintf("done\n");
		}
		
		if (Disc_Init() < 0) {
			return -4;
		}
		if (Disc_Open() < 0) {
			return -6;
		}
		if (WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer) < 0) {
			return -8;
		}
	}
	

	gprintf("Setting low memory\n");
	
	/* Setup low memory */;
	__Disc_SetLowMem();

	gprintf("Select video mode\n");

	/* Select an appropriate video mode */
	__Disc_SelectVMode(vidMode);

	gprintf("Running apploader\n");

	/* Run apploader */
	ret = Apploader_Run(&p_entry, cheat != 0, vidMode, vmode, vipatch, countryString, patchVidMode);
	if (ret < 0) {
		gprintf("Failed to run apploader\n");
		return ret;
	}

	gprintf("Set video mode\n");

	/* Set an appropriate video mode */
	__Disc_SetVMode();

	if (cheat != 0 && hooktype != 0)
	{
		ocarina_do_code();
	}

	gprintf("Set time\n");

	/* Set time */
	__Disc_SetTime();

	gprintf("Reset video...");

	/* This prevent a green screen (or a flash of green before loading the game) */
	VIDEO_SetBlack(TRUE);
	gprintf("flushing...");
	VIDEO_Flush();
	gprintf("wait for sync...");
	VIDEO_WaitVSync();
	gprintf("wait for sync...");
	VIDEO_WaitVSync();
	gprintf("\n");

	u8 temp_data[4];

	gprintf("Shutting down wii systems\n");

	// fix for PeppaPig
	memcpy((char *) &temp_data, (void*)0x800000F4,4);

	/* Shutdown IOS subsystems */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	// fix for PeppaPig
	memcpy((void*)0x800000F4,(char *) &temp_data, 4);

	appentrypoint = (u32) p_entry;

	gprintf("Current IOS: %d\n", IOS_GetVersion());

	gprintf("Starting game\n");

	if (cheat != 0)
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"mtctr %r3\n"
			"bctr\n"
		);
	}
	else
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}

	return 0;
}

s32 Disc_OpenPartition(u32 mode, u8 *id)
{
	u64 offset;
	return Disc_OpenPartitionO(mode, id, &offset);
}

s32 Disc_OpenPartitionO(u32 mode, u8 *id, u64 *offset)
{
	if (Disc_Open() < 0)
		return -2;
	if (__Disc_FindPartition(offset) < 0)
		return -3;
	if (WDVD_OpenPartition(*offset, 0, 0, 0, Tmd_Buffer) < 0)
		return -4;
	return 0;
}

s32 Disc_OpenPartitionSimple(u64 offset)
{
	return WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer);
}

s32 Disc_WiiBoot(u8 vidMode, const u8 *cheat, u32 cheatSize, bool vipatch, bool countryString, u8 patchVidModes)
{
	u64 offset;
	s32 ret;

	/* Find game partition offset */
	ret = __Disc_FindPartition(&offset);
	if (ret < 0) {
		gprintf("Cannot find partition offset: %d\n", ret);
		return ret;
	}

	/* Boot partition */
	return Disc_BootPartition(offset, vidMode, cheat, cheatSize, vipatch, countryString, patchVidModes);
}

#define BC 		0x0000000100000100ULL
// #define MIOS 	0x0000000100000101ULL

// static tikview view ATTRIBUTE_ALIGN(32);

s32 Disc_GCBoot()
{
/*
	gprintf("Changing value at address 0xCC03024\n");
	*(volatile unsigned int *)0xCC003024 |= 7;
		
	gprintf("Retrieving Ticket for BC...");
	int retval = ES_GetTicketViews(BC, &view, 1);

	if (retval != 0) {
		gprintf("failed %d\n",retval);
		return -1;
	}
	gprintf("done\n");
	ghexdump(&view, sizeof(tikview));
*/
	gprintf("Disconnecting Wiimotes\n");
	/* Disconnect Wiimote */
    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

	gprintf("Setting video to black...");

	VIDEO_SetBlack(TRUE);
	gprintf("flushing...");
	VIDEO_Flush();
	gprintf("wait for sync...");
	VIDEO_WaitVSync();
	gprintf("wait for sync...");
	VIDEO_WaitVSync();
	gprintf("\n");

	gprintf("Launching title\n");
	WII_LaunchTitle(BC);
/*
	retval = ES_LaunchTitle(BC, &view);
	gprintf("Launching title failed: %d\n",retval);	
	while(1);
*/	
	return 0;
}

s32 Disc_ClosePartition(void)
{
	return WDVD_ClosePartition();
}