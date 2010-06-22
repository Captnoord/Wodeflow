#include <ogc/system.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include "video.hpp"
#include "menu/menu.hpp"
#include "loader/disc.h"
#include "loader/fs.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/wdvd.h"
#include "text.hpp"
#include "gecko.h"

extern int use_dvdx;

extern "C"
{
    extern void __exception_setreload(int t);
}

extern const u8 wait_png[];
extern const u8 wait_disc_png[];

extern bool geckoinit;
extern int mainIOS;
extern int mainIOSminRev;
extern int mainIOSRev;

extern "C" {
	int InitDVD();
}

int old_main()
{
	geckoinit = InitGecko();
//	__exception_setreload(5);
	use_dvdx = 1;
	
	InitDVD();

	SYS_SetArena1Hi((void *)0x81200000);	// See loader/apploader.c
	CVideo vid;
	
	gprintf("WodeFlow started, running with ios %d\n", IOS_GetVersion());

	bool dipOK = false;
	int ret = 0;
	bool hbc;
	bool wodeOK = false;
		
	// MEM2 usage :
	// 36 MB for general purpose allocations
	// 12 MB for covers (and temporary buffers)
	// adds 15 MB from MEM1 to obtain 27 MB for covers (about 150 HQ covers on screen)
	MEM2_init(36, 12);	// Max ~48

	// Launched through the HBC?
    hbc = *((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858;

	// Init video
	vid.init();
	
	// Init
	STexture texWait;
	texWait.fromPNG(wait_png, GX_TF_RGB565, ALLOC_MALLOC);
	vid.waitMessage(texWait);
	Sys_Init();
	Sys_ExitToWiiMenu(true);

	Fat_Mount();

	WPAD_Init();
	WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);

	dipOK = Disc_Init() >= 0;
	if (dipOK) {
		u32 status = 0;
		if (WDVD_GetCoverStatus(&status) != 0 || (status & 2) == 0) {
			// WDVD_WaitForDisc();
			
			STexture texWaitForDisc;
			texWaitForDisc.fromPNG(wait_disc_png, GX_TF_RGB565, ALLOC_MALLOC);
			vid.waitMessage(texWaitForDisc);
			texWaitForDisc.data.release();
			do
			{
				WPAD_ScanPads();
				s32 padsState = WPAD_ButtonsDown(0);
				if ((padsState & WPAD_BUTTON_B) != 0)
					break;
				usleep(100 * 1000);
			} while (WDVD_GetCoverStatus(&status) != 0 || (status & 2) == 0);
			if ((status & 2) == 0) return -2;
			
			InitDVD();
		}
	}
	
	wodeOK = WBFS_Init() >= 0;
	vid.waitMessage(texWait);
	texWait.data.release();
	
	MEM2_takeBigOnes(true);
	do
	{
		CMenu menu(vid);
		menu.init(hbc);

		if (!dipOK)
			menu.error(L"Could not initialize DIP module!");
		else if (!wodeOK)
			menu.error(L"Wode not found.");
		else
		{
			ret = menu.main();
		}
		vid.cleanup();
	} while (ret == 1);
	
	LaunchISO(0, 0);
	CloseWode();
	return ret;
};

int main(int, char **)
{
	Sys_Exit(old_main());
	return 0;
}
