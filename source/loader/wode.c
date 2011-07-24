#include <gccore.h>        // Wrapper to include common libogc headers
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <di/di.h>
#include <ogc/system.h>
#include <sys/unistd.h>
#include <ogc/dvd.h>
#include "disc.h"
#include "wode.h"
#include "gecko.h"
#include "logger.h"

#define MAX_FAVORITES 8

//#define WODE_MAGIC_DVDx			(0xFF574800)		/* 0xFF,WO0 -> Used for D0 reads (DVDx) */ 
#define WODE_MAGIC_IOS			(0xFF5748E8)		/* 0xFF,WOD -> Used for A8 reads (IOS, cIOS) */
#define WODE_EXIT_REMOTE		(0xFF000000)
#define WODE_GET_NUM_PARTS		(0x10000000)
#define WODE_GET_PART(P)		(0x11000000 | ((P & 0xFF) << 16))
#define WODE_GET_NUM_ISOS(P)	(0x12000000 | ((P & 0xFF) << 16))
#define WODE_GET_ISO(P1,P2)		(0x13000000 | ((P1 & 0xFF) << 16) | ((P2 & 0xFF) << 8) | ((P2 & 0x3F00) >> 6))

// Without DVDx
#define WODE_GET_ISO(P1,P2)		(0x13000000 | ((P1 & 0xFF) << 16) | ((P2 & 0xFF) << 8) | ((P2 & 0x3F00) >> 6))

// With DVDx
#define WODE_GET_ISO_A(P)		(0x40000000 | ((P & 0xFF) << 16))
#define WODE_GET_ISO_B(P)		(0x41000000 | ((P >> 5) << 16) | (((P & 0x1F) << 3) << 8))

#define WODE_GET_FAVORITE_INFO(P)		(0x14000000 | ((P & 0xFF) << 16))
#define WODE_ERASE_FAVORITE(P)			(0x15000000 | ((P & 0xFF) << 16))
#define WODE_GET_NUM_FAVORITES			(0x16000000)
#define WODE_SET_FAVORITE_FAVE(P)		(0x17000000 |  ((P & 0xFF) 	 << 16))
#define WODE_SET_FAVORITE_PART(P)		(0x18000000 |  ((P & 0xFF)   << 16))
#define WODE_SET_FAVORITE_ISO(P)		(0x19000000 |  ((P & 0xFFFF) << 8))

#define WODE_SET_REGION_HACK(P)			(0x20000000 | ((P & 0xFF) << 16))
#define WODE_SET_UPDATE_BLOCK_HACK(P)	(0x21000000 | ((P & 0xFF) << 16))
#define WODE_SET_AUTOBOOT_HACK(P)		(0x22000000 | ((P & 0xFF) << 16))
#define WODE_SET_RELOAD_HACK(P)			(0x23000000 | ((P & 0xFF) << 16))
#define WODE_SET_WII_REGION(P)			(0x24000000 | ((P & 0xFF) << 16))
#define WODE_SET_DISPLAY(P)				(0x25000000 | ((P & 0xFF) << 16))

#define WODE_WRITE_SETTINGS				(0x2F000000)

#define WODE_GET_SETTINGS		(0x30000000)
#define WODE_GET_VERSIONS		(0x31000000)

//#define WODE_GET_JSTICK			(0x40000000)

#define WODE_GOTO_FLAT_MODE				(0x80000000)

// Without DVDx
#define WODE_LAUNCH_GAME_PART(P)		(0x81000000 |  ((P & 0xFF) 	 << 16))
//#define WODE_LAUNCH_GAME_ISO(P)			(0x82000000 |  ((P & 0xFFFF) <<  8))
#define WODE_LAUNCH_GAME_ISO(P)			(0x82000000 |  ((P & 0xFFFF) <<  11))
//#define WODE_LAUNCH_GAME_ISO(P)			(0x82000000 |  ((P >> 5) << 16) | (((P & 0x1F) << 3) << 8))

// With DVDx
//#define WODE_LAUNCH_GAME_PART_DVDx(P)		(0x81000000 |  ((P & 0xFF) 	 << 16))
//#define WODE_LAUNCH_GAME_ISO_DVDx(P)			(0x82000000 |  ((P >> 5) << 16) | (((P & 0x1F) << 3) << 8))

//#define DEBUG_WODE
//#define gprintf log_printf

static u8 dvdbuffer[0x8000] ATTRIBUTE_ALIGN (32);    // One Sector

static const char di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";

int use_dvdx = 1;
int is_wii_disc = 0;
dvdcmdblk cmdblk;

int DVDW_Read(void *buf, uint32_t len, uint32_t offset)
{
	if (use_dvdx) {
		uint32_t nlen = (len >> 11) + 1;
		
		memset(buf, 0, len);
		
#ifdef DEBUG_WODE		
		gprintf("Sending DI command @ offset: 0x%08x, len: %d\n", offset, len);
#endif		
		int ret = DI_ReadDVD(buf, nlen, offset >> 11);
		if (ret < 0) {
#ifdef DEBUG_WODE
			gprintf("failed: %d\n", ret);
#endif
			return ret;
		}
#ifdef DEBUG_WODE		
		gprintf("Buffer:");
		ghexdump(buf, len);
#endif
		return ret;
	}
	
	return DVD_ReadPrio(&cmdblk, buf, len, offset, 2);
}

//--------------------------------------------------------------------------
int DVDx_InitDVD()
{
	DI_UseCache(false);
	DI_LoadDVDX(false);
	
	DI_Init();
	DI_Mount();
	while(DI_GetStatus() & DVD_INIT);
	return (DI_GetStatus() & DVD_READY) != 0 ? 0 : -1;
}

int InitDVD()
{
	if (use_dvdx) {
		return DVDx_InitDVD();
	}
#ifdef DEBUG_WODE		
	gprintf("DVD init...\n");
#endif
	DVD_Init();
#ifdef DEBUG_WODE		
	gprintf("done\n");
#endif
	return 0;
}

void DeinitDVD()
{
	if (use_dvdx) {
		DI_Close();
	}
}

int OpenWode( void )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_MAGIC)\n");
#endif

	int ret = DVDW_Read(dvdbuffer, 0x20, WODE_MAGIC_IOS);
	if (ret != 0) {
		return -2;
	}

	if(	dvdbuffer[0] == 'W' &&
		dvdbuffer[1] == 'O' &&
		dvdbuffer[2] == 'D' &&
		dvdbuffer[3] == 'E')
		return 0;
	return -1;
}

int CloseWode( void )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_EXIT_REMOTE)\n");
#endif
	DVDW_Read(dvdbuffer, 0x20, WODE_EXIT_REMOTE);
	if (use_dvdx) {
		DI_Close();
	}
	return 0;
}
/*
unsigned long GetJoystick( void )
{
	dvdcmdblk cmdblk;
	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_GET_JSTICK, 2) != 0){
		return -1;
	}
	return *((unsigned long*)dvdbuffer);
}
*/

unsigned long GetNumPartitions( void )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_NUM_PARTS)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_NUM_PARTS) != 0){
		return -1;
	}
	unsigned long *ptr = (unsigned long *) dvdbuffer;
	return *ptr;
}

unsigned long GetNumISOs(unsigned long partition_idx)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_NUM_ISOS(%d))\n", partition_idx);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_NUM_ISOS(partition_idx)) != 0){
		return -1;
	}
	unsigned long *ptr = (unsigned long *) dvdbuffer;
	return *ptr;
}

int GetPartitionInfo(unsigned long partition_idx, PartitionInfo_t* PartitionInfo)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_PART(%d))\n", partition_idx);
#endif
	if(DVDW_Read(dvdbuffer, 0x80, WODE_GET_PART(partition_idx)) != 0){
		return -1;
	}
	memcpy(PartitionInfo->name,dvdbuffer,64);
	PartitionInfo->NumISOs 			= *((unsigned long*)&dvdbuffer[64]);
	PartitionInfo->partition_type 	= *((unsigned long*)&dvdbuffer[68]);
	PartitionInfo->partition_mode 	= *((unsigned long*)&dvdbuffer[72]);
	return 0;	
}

int GetISOInfo(unsigned long partition_idx, unsigned long iso_idx, ISOInfo_t * ISOInfo)
{
	if (use_dvdx) {
#ifdef DEBUG_WODE		
		gprintf("Send DI command (WODE_GET_ISO_A(%d))\n", partition_idx);
#endif
		if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_ISO_A(partition_idx)) != 0){
			return -1;
		}
#ifdef DEBUG_WODE		
		gprintf("Send DI command (WODE_GET_ISO_B(%d))\n", iso_idx);
#endif
		if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_ISO_B(iso_idx)) != 0){
			return -1;
		}
	} else {
		if (DVDW_Read(dvdbuffer, 0x80, WODE_GET_ISO(partition_idx, iso_idx)) != 0) {
			return -1;
		}
	}
	memcpy(ISOInfo->name,dvdbuffer,64);
	ISOInfo->iso_type 		= *((unsigned long*)&dvdbuffer[64]);
	ISOInfo->iso_region 	= *((unsigned long*)&dvdbuffer[68]);
	memcpy(ISOInfo->header,dvdbuffer + 72, 8);
	return 0;
}

unsigned long GotoFlatMode( void )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GOTO_FLAT_MODE)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GOTO_FLAT_MODE) != 0){
		return -1;
	}
	CloseWode();
	return 0;
}

unsigned long GetMaxFavorites( void )
{
	return MAX_FAVORITES;
}

int GetFavoriteInfo(unsigned long index, FavoriteInfo_t * favoriteInfo)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_FAVORITE_INFO(%d))\n", index);
#endif
	if(DVDW_Read(dvdbuffer, 0x80, WODE_GET_FAVORITE_INFO(index)) != 0){
		return -1;
	}

	favoriteInfo->state     = *((unsigned long*)&dvdbuffer[0]);
	favoriteInfo->partition = *((unsigned long*)&dvdbuffer[4]);
	favoriteInfo->iso       = *((unsigned long*)&dvdbuffer[8]);

	memcpy(favoriteInfo->name,&dvdbuffer[12],64);
	favoriteInfo->iso_type 	 = *((unsigned long*)&dvdbuffer[76]);
	favoriteInfo->iso_region = *((unsigned long*)&dvdbuffer[80]);

	return 0;
}

int EraseFavorite(unsigned long idx)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_ERASE_FAVORITE(%d))\n", idx);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_ERASE_FAVORITE(idx)) != 0){
		return -1;
	}
	return 0;
}

int GetNumFavorites( void )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_NUM_FAVORITES)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_NUM_FAVORITES) != 0){
		return -1;
	}
	unsigned long *ptr = (unsigned long *) dvdbuffer;
	return *ptr;
}

int InsertFavorite(unsigned long IsoIndex, unsigned long favorite_idx)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_SET_FAVORITE_FAVE(%d))\n", favorite_idx);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_SET_FAVORITE_FAVE(favorite_idx)) != 0){
		return -1;
	}

	/* 256 partitions should be enough */
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_SET_FAVORITE_PART(%d))\n", favorite_idx);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_SET_FAVORITE_PART(favorite_idx)) != 0){
		return -2;
	}

#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_SET_FAVORITE_ISO(%d))\n", IsoIndex);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_SET_FAVORITE_ISO(IsoIndex)) != 0){
		return -3;
	}

	return 0;	
}

unsigned long SaveSettings( void )
{
/*
	dvdcmdblk cmdblk;

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_REGION_HACK(MENU_GetRegion()), 2) != 0){	//20
		printf ("Error -> SetRegion\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_UPDATE_BLOCK_HACK(MENU_GetBlockUpdates()), 2) != 0){  //21
		printf ("Error -> SetBlockUpdates\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_AUTOBOOT_HACK(MENU_GetAutoStart()), 2) != 0){	//22
		printf ("Error -> SetAutoStart\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_RELOAD_HACK(MENU_GetAutoload()), 2) != 0){	//23
		printf ("Error -> SetWiiRegion\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_WII_REGION(MENU_GetWiiRegion()), 2) != 0){		//24
		printf ("Error -> SetAutoload\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_SET_DISPLAY(MENU_GetDisplay()), 2) != 0){		//24
		printf ("Error -> SetDisplay\n");
		return -1
	}

	if(DVDW_ReadPrio (&cmdblk, dvdbuffer, 0x20 , WODE_WRITE_SETTINGS, 2) != 0){
		printf ("Error -> Wite Settings\n");
		return -1
	}

	if(	(MENU_GetRegion() 		== dvdbuffer[0]) &&
		(MENU_GetBlockUpdates() == dvdbuffer[1]) &&
		(MENU_GetAutoStart() 	== dvdbuffer[2]) &&
		(MENU_GetDisplay() 		== dvdbuffer[3]) &&
		(MENU_GetWiiRegion() 	== dvdbuffer[4]) &&
		(MENU_GetAutoload() 	== dvdbuffer[5]))
		{
			DrawBitmap(160, 150, 7);	
			sleep(2);
			
		}
	else
		{
			DrawBitmap(160, 150, 8);	
			sleep(2);
		}
*/
	return 0;
}

int GetSettings( s_user_settings * settings )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_SETTINGS)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_SETTINGS) != 0){
		return -1;
	}

	settings->s_region_hack		= dvdbuffer[0];
	settings->s_update_blocker	= dvdbuffer[1];
	settings->s_autoboot		= dvdbuffer[2];
	settings->s_display			= dvdbuffer[3];
	settings->s_wii_region		= dvdbuffer[4];
	settings->s_autoload		= dvdbuffer[5];
	return 0;
}

int GetVersions( device_versions * versions )
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_VERSIONS)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_VERSIONS) != 0){
		return -1;
	}

	versions->loader_version   = dvdbuffer[0];
	versions->loader_version <<= 8;
	versions->loader_version  |= dvdbuffer[1];

	versions->wode_version   = dvdbuffer[2];
	versions->wode_version <<= 8;
	versions->wode_version  |= dvdbuffer[3];

	versions->fpga_version   = dvdbuffer[4];
	versions->fpga_version <<= 8;
	versions->fpga_version  |= dvdbuffer[5];

	versions->hw_version	 = dvdbuffer[6];

	return 0;
}

int LaunchISO(unsigned long Partition, unsigned long Iso)
{
#ifdef DEBUG_WODE		
	gprintf("Mounting iso %d on partition %d\n", Iso, Partition);
	gprintf("Send DI command (WODE_LAUNCH_GAME_PART(%d))\n", Partition);
#endif
	if(DVDW_Read(dvdbuffer, 0x20,WODE_LAUNCH_GAME_PART(Partition)) != 0){
		return -1;
	}
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_LAUNCH_GAME_ISO(%d))\n", Iso);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_LAUNCH_GAME_ISO(Iso)) != 0) {
		return -2;
	}
	return 0;
}

int SetAutoBoot(unsigned long autoboot)
{
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_GET_SETTINGS)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_GET_SETTINGS) != 0){
		return -1;
	}
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_SET_RELOAD_HACK(%d))\n", autoboot);
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_SET_RELOAD_HACK(autoboot)) != 0){	//23
		return -2;
	}
#ifdef DEBUG_WODE		
	gprintf("Send DI command (WODE_WRITE_SETTINGS)\n");
#endif
	if(DVDW_Read(dvdbuffer, 0x20, WODE_WRITE_SETTINGS) != 0){
		return -3;
	}	
	return 0;
}

unsigned long GetSelectedISO( void )
{
	return 0;
}

int GetTotalISOs( void )
{
	PartitionInfo_t PartitionInfo;
	unsigned long i;
	unsigned long NumPartitions = GetNumPartitions( );
	int TotalISOs = 0;
	
	for(i=0;i<NumPartitions;i++)
	{
		GetPartitionInfo(i, &PartitionInfo);
		TotalISOs += PartitionInfo.NumISOs;
	}
	return TotalISOs;
}
