#ifndef _WBFS_H_
#define _WBFS_H_

extern char wbfs_fs_drive[16];

typedef void (*progress_callback_t)(int status,int total,void *user_data);

/* Prototypes */
s32 WBFS_Init();
s32 WBFS_Open(void);
s32 WBFS_Format(u32, u32);
s32 WBFS_GetCount(u32 *);
s32 WBFS_GetHeaders(void *, u32, u32);
s32 WBFS_CheckGame(u8 *);
s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data);
s32 WBFS_RemoveGame(u8 *);
//s32 WBFS_DiskSpace(f32 *, f32 *);

s32 WBFS_OpenPart(u32 part_idx, char *partition);
s32 WBFS_OpenNamed(char *partition);

s32 WBFS_OpenDisc(u8 *, unsigned long game_idx, unsigned long game_part);
void WBFS_CloseDisc();

bool WBFS_Close();

/**
 * populate the game list with disc's
 *
 * @param[in][out] game_list vector list of game disc structures
 * @return returns zero when successful and non zero when unsuccessful
 */
s32 WBFS_populate_game_list(std::vector<struct discHdr> & game_list);

unsigned long WBFS_GetCurrentPartition();
unsigned long WBFS_GetPartitionCount();
s32 WBFS_GetPartitionName(u32, char *, u32*);
u32 WBFS_GetDefaultPartition();

bool WBFS_IsReadOnly(void);
f32 WBFS_EstimeGameSize(void);

#endif
