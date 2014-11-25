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
#include <vector>

#include "wbfs.h"
#include "wdvd.h"
#include "sys.h"
#include "disc.h"

#include "wode.h"
#include "logger.h"

long partition_idx = -1;
void *discHeaders = NULL;

s32 WBFS_Init()
{
	s32 ret = -1;

	/* Try to mount device */
	ret = OpenWode();

	if (ret == 0)
		goto out;
		
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
	
	if (!strncmp("al", partition, 3)) {
		partition_idx = 0xFFFF; // enum of all... bleh...
		return 0;
	}
	
	for (i = 1; i < GetNumPartitions(); i++) {
		if (GetPartitionInfo(i, &t) == 0 && memcmp(t.name, partition, strlen(t.name)) == 0) {
			partition_idx = i;
			return 0;
		}
	}

	return -1;
}

s32 WBFS_GetCountEx(u32 *count, s32 part, u32 * index)
{
	int idx, ret, cnt;
	int amount = 0;
	char partname[255];
	u32 partnamelen = 0;
	
	/* No device open */
	if (part == -1)
		return -1;
		
	ret = 0;
		
	/* simply don't list ram1 */
	WBFS_GetPartitionName(part, (char*)partname, &partnamelen);
	partname[partnamelen] = '\0';
	if (!strncmp("ram", partname, 3))
		return 0;
	
	/* Get list length */
	cnt = GetNumISOs( part );
	
//	log_printf("part: %u [%s] contains: %u iso's\n", part, partname, cnt);
	
	// Get all discHdrs
	for (idx = 0; idx < cnt; idx++) {

		struct discHdr *ptr = &((struct discHdr *)discHeaders)[(*index)++];

		// Get header
		ISOInfo_t iso;
		ret = GetISOInfo(part, idx, &iso);
		if (ret != 0)
			goto exit;

		if (iso.iso_type == TYPE_GC && iso.header[6] != 0)	// Skip this game, since it's a multi-disc game
			continue;										// and this disc isn't the first one
			
		if (iso.iso_type == TYPE_UNKNOWN)					// Filter unknown iso types
			continue;

		memset(ptr, 0, sizeof(struct discHdr));
		memcpy(ptr->id, iso.header, 6);
		
		strncpy(ptr->title, iso.name, sizeof(ptr->title) - 1);
		ptr->game_idx = idx;
		ptr->game_part = part;
		ptr->magic = iso.iso_type == TYPE_GC ? GC_MAGIC : WII_MAGIC;
		
//		log_printf("\t[%u] iso: %u part: %u name:%s type:%u\n", (*index), idx, part, iso.name, iso.iso_type);
		
		amount++;
	}
	
exit:
	
	*count = amount;

	return ret;
}

s32 WBFS_GetCount(u32 *count)
{
	u32 i = 0;
	u32 temp_count;
	u32 part_count = GetNumPartitions();
	u32 index = 0;
	
	int total_iso_count = GetTotalISOs();
	
	discHeaders = malloc( total_iso_count * sizeof(struct discHdr));
	memset(discHeaders, 0, total_iso_count * sizeof(struct discHdr));
	
	*count = 0;
	for (i = 0; i < part_count; i++)
	{
		temp_count = 0;
		WBFS_GetCountEx(&temp_count, i, &index);
		*count += temp_count;
	}
	
	return 0;
}


s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	if (discHeaders == NULL)
		return -1;
	
	memcpy(outbuf, discHeaders, cnt * len);
	free(discHeaders);
	discHeaders = NULL;
	return 0;
}

s32 WBFS_populate_game_list_ex(std::vector<struct discHdr> & game_list, int part)
{
	int idx, ret, cnt;
	char partname[255];
	u32 partnamelen = 0;
	
	/* No device open */
	if (part == -1)
	{
		printf("unable to open partition\n");
		return -1;
	}
	
	/* simply don't list "ram*" */
	WBFS_GetPartitionName(part, (char*)partname, &partnamelen);
	partname[partnamelen] = '\0';
	if (!strncmp("ram", partname, 3))
		return 0;
	
	/* Get list length */
	cnt = GetNumISOs( part );
	
	//log_printf("part: %u [%s] contains: %u iso's\n", part, partname, cnt);
	
	// Get all discHdrs
	for (idx = 0; idx < cnt; idx++) {

		// Get header
		ISOInfo_t iso;
		ret = GetISOInfo(part, idx, &iso);
		if (ret != 0)
			return ret;

		if (iso.iso_type == TYPE_GC && iso.header[6] != 0)	// Skip this game, since it's a multi-disc game
			continue;										// and this disc isn't the first one

		if (iso.iso_type == TYPE_UNKNOWN)					// Filter unknown iso types
			continue;

		struct discHdr disc;
		memcpy(disc.id, iso.header, 6);
		
		strncpy(disc.title, iso.name, sizeof(disc.title) - 1);
		disc.game_idx = idx;
		disc.game_part = part;
		disc.magic = iso.iso_type == TYPE_GC ? GC_MAGIC : WII_MAGIC;
		game_list.push_back(disc);

		//log_printf("\t[%u] iso: %u part: %u name:%s type:%u\n", (*index), idx, part, iso.name, iso.iso_type);
	}
	
	return 0;
}

s32 WBFS_populate_game_list(std::vector<struct discHdr> & game_list)
{
	int total_iso_count;
	int part_count = GetNumPartitions();
	int i;
	
	game_list.clear();
	
	/* list all iso's */
	if (partition_idx == 0xFFFF) {
	
		total_iso_count = GetTotalISOs();
	
		game_list.reserve(total_iso_count);
	
		for (i = 0; i < part_count; i++)
			WBFS_populate_game_list_ex(game_list, i);
			
	}
	else /* list one specific partition */
	{
	
		total_iso_count = GetNumISOs(partition_idx);
		
		game_list.reserve(total_iso_count);
		
		WBFS_populate_game_list_ex(game_list, partition_idx);
	}
	
	/**
	 * @note I don't shrink the game_list buffer to the correct size. I don't do that because it
	 * 		 the std::vector only has a resize function which does someting else.
	 */
	return 0;
}

s32 WBFS_OpenDisc(u8 * /*gameId*/, unsigned long game_idx, long part)
{
	/* No device open */
	if (part == -1)
		return -1;

	/* Open disc */
	return LaunchISO(part, game_idx);
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

unsigned long WBFS_GetCurrentPartition()
{
	return partition_idx;
}

unsigned long WBFS_GetPartitionCount()
{
	return GetNumPartitions();
}

s32 WBFS_GetPartitionName(u32 index, char *buf, u32* len)
{
	PartitionInfo_t t;
	memset(&t, 0, sizeof(t));
	if (GetPartitionInfo(index, &t) == 0)
	{
		(*len) = strlen(t.name);
		strncpy(buf, t.name, strlen(t.name));
		buf[strlen(t.name)] = '\0';
		return 0;
	}
	return -1;
}

u32 WBFS_GetDefaultPartition()
{
	return WBFS_GetPartitionCount() == 1 ? 0 : 1; // There is always one partition, it's the internal RAM partition with the GC config disc.
}

bool WBFS_IsReadOnly(void)
{
	PartitionInfo_t t;
	if (partition_idx != -1 && GetPartitionInfo(partition_idx, &t) == 0) {
		return t.partition_mode == pm_read_write;
	}
	return true;
}
