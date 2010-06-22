#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <ctype.h>
#include <sdcard/wiisd_io.h>
#include <string.h>

#include "wbfs.h"
#include "wdvd.h"
#include "sys.h"
#include "disc.h"

#include "wode.h"

unsigned long partition_idx = -1;
void *discHeaders = NULL;

s32 WBFS_Init()
{
	s32 ret = -1;

	/* Try to mount device */
	ret = OpenWode();

	if (ret == 0) {
		goto out;
	}
		
	/* Sleep 1 second */
	sleep(1);

out:
	return ret;
}

bool WBFS_Close()
{
	partition_idx = -1;
	return CloseWode() == 0;
}

s32 WBFS_Open(void)
{
	return 0;
}

s32 WBFS_OpenPart(u32 part_idx, char *partition)
{
	partition_idx = part_idx;
	
	PartitionInfo_t t;
	memset(&t, 0, sizeof(PartitionInfo_t));
	if (GetPartitionInfo(partition_idx, &t) == 0) {
		strncpy(partition, t.name, strlen(t.name));
		return 0;
	}
	return -1;
}

s32 WBFS_OpenNamed(char *partition)
{
	unsigned long i;
	PartitionInfo_t t;
	for (i = 0; i < GetNumPartitions(); i++) {
		if (GetPartitionInfo(i, &t) == 0 && memcmp(t.name, partition, strlen(t.name)) == 0) {
			partition_idx = i;
			return 0;
		}
	}
	return -1;
}


s32 WBFS_GetCount(u32 *count)
{
	/* No device open */
	if (partition_idx == -1)
		return -1;

	/* Get list length */
	int idx, ret, cnt = GetNumISOs(partition_idx);

	discHeaders = malloc(cnt * sizeof(struct discHdr));
	memset(discHeaders, 0, cnt * sizeof(struct discHdr));

	int amount = 0;

	// Get all discHdrs
	for (idx = 0; idx < cnt; idx++) {
		struct discHdr *ptr = &((struct discHdr *)discHeaders)[amount];

		// Get header
		ISOInfo_t iso;
		ret = GetISOInfo(partition_idx, idx, &iso);
		if (ret != 0)
			return ret;

		if (iso.iso_type == TYPE_GC && iso.header[6] != 0)  // Skip this game, since it's a multi-disc game
			continue;										 // and this disc isn't the first one
			
		if (iso.iso_type == TYPE_UNKNOWN)	// Filter unknown iso types
			continue;

		memset(ptr, 0, sizeof(struct discHdr));
		memcpy(ptr->id, iso.header, 6);
		strncpy(ptr->title, iso.name, sizeof(ptr->title) - 1);
		ptr->game_idx = idx;
		ptr->magic = iso.iso_type == TYPE_GC ? GC_MAGIC : WII_MAGIC;
		
		amount++;
	}
	
	discHeaders = realloc(discHeaders, amount * sizeof(struct discHdr));
	*count = amount;

	return 0;
}

s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	if (discHeaders == NULL) {
		return -1;
	}
	
	memcpy(outbuf, discHeaders, cnt * len);
	free(discHeaders);
	discHeaders = NULL;
	return 0;
	
/*
	u32 idx; //, size;
	s32 ret;

	if (partition_idx == -1)
		return -1;

	for (idx = 0; idx < cnt; idx++) {
		struct discHdr *ptr = &((struct discHdr *)outbuf)[idx];

		// Get header
		ISOInfo_t iso;
		ret = GetISOInfo(partition_idx, idx, &iso);
		if (ret != 0)
			return ret;

		memset(ptr, 0, sizeof(struct discHdr));
		memcpy(ptr->id, iso.header, 6);
		strncpy(ptr->title, iso.name, sizeof(ptr->title) - 1);
		ptr->game_idx = idx;
		ptr->magic = iso.iso_type == TYPE_GC ? GC_MAGIC : WII_MAGIC;
	}
	return 0;
*/
}

s32 WBFS_CheckGame(u8 *discid)
{
	// TODO: If the game exists, return 1, otherwise 0
	return 1;
}

s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data)
{
	/* No device open */
	if (partition_idx == -1)
		return -1;

	// TODO: Add game

	return -1;
}

s32 WBFS_RemoveGame(u8 *discid)
{
	/* No device open */
	if (partition_idx == -1)
		return -1;

	// TODO: Remove game

	return -1;
}

s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
	/* No device open */
	if (partition_idx == -1)
		return -1;

	// TODO: Get free discspace of partition

	return -1;
}

s32 WBFS_OpenDisc(u8 *gameId, unsigned long game_idx)
{
	/* No device open */
	if (partition_idx == -1)
		return -1;

	/* Open disc */
	return LaunchISO(partition_idx, game_idx);
}

void WBFS_CloseDisc()
{
	/* No device open */
	if (partition_idx == -1)
		return;

	/* Close disc */
//	LaunchISO(partition_idx, -1);
}

typedef struct {
	u8 filetype;
	char name_offset[3];
	u32 fileoffset;
	u32 filelen;
} __attribute__((packed)) FST_ENTRY;

static inline u32 _be32(const u8 *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

char *fstfilename2(FST_ENTRY *fst, u32 index)
{
	u32 count = _be32((u8*)&fst[0].filelen);
	u32 stringoffset;
	if (index < count)
	{
		//stringoffset = *(u32 *)&(fst[index]) % (256*256*256);
		stringoffset = _be32((u8*)&(fst[index])) % (256*256*256);
		return (char *)((u32)fst + count*12 + stringoffset);
	} else
	{
		return NULL;
	}
}

unsigned long WBFS_GetCurrentPartition() {
	return partition_idx;
}

unsigned long WBFS_GetPartitionCount() {
	return GetNumPartitions();
}

s32 WBFS_GetPartitionName(u32 index, char *buf) {
	PartitionInfo_t t;
	if (GetPartitionInfo(index, &t) == 0) {
		strncpy(buf, t.name, strlen(t.name));
		return 0;
	}
	return -1;
}

u32 WBFS_GetDefaultPartition() {
	return WBFS_GetPartitionCount() == 1 ? 0 : 1; // There is always one partition, it's the internal RAM partition with the GC config disc.
}

bool WBFS_IsReadOnly(void) {
	PartitionInfo_t t;
	if (partition_idx != -1 && GetPartitionInfo(partition_idx, &t) == 0) {
		return t.partition_mode == pm_read_write;
	}
	return true;
}

f32 WBFS_EstimeGameSize(void) {
	return 0;
//    return wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
}
