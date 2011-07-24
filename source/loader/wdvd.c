#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#include "gecko.h"

/* Constants */
#define IOCTL_DI_READID		0x70
#define IOCTL_DI_READ		0x71
#define IOCTL_DI_WAITCVRCLOSE	0x79
#define IOCTL_DI_GETCOVER	0x88
#define IOCTL_DI_RESET		0x8A
#define IOCTL_DI_OPENPART	0x8B
#define IOCTL_DI_CLOSEPART	0x8C
#define IOCTL_DI_UNENCREAD	0x8D
#define IOCTL_DI_SEEK		0xAB
#define IOCTL_DI_STOPLASER	0xD2
#define IOCTL_DI_OFFSET		0xD9
#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_DISABLERESET	0xF6

//#define DEBUG_WDVD

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);

static const char di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";
static s32 di_fd = -1;

s32 WDVD_Init(void)
{
	/* Open "/dev/di" */
	
	if (di_fd < 0) {
#ifdef DEBUG_WDVD
		gprintf("Opening /dev/di device...");
#endif
		di_fd = IOS_Open(di_fs, 0);
		if (di_fd < 0) {
#ifdef DEBUG_WDVD
			gprintf("failed: %d\n", di_fd);
#endif
			return di_fd;
		}
#ifdef DEBUG_WDVD
		gprintf("done: %d\n", di_fd);
#endif
	}
	return 0;
}

s32 WDVD_Close(void)
{
	/* Close "/dev/di" */
	if (di_fd >= 0) {
#ifdef DEBUG_WDVD
		gprintf("Closing /dev/di\n");
#endif
		IOS_Close(di_fd);
		di_fd = -1;
	}

	return 0;
}

s32 WDVD_GetHandle(void)
{
	/* Return di handle */
	return di_fd;
}

s32 WDVD_Reset(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Reset drive */
	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_RESET (%x), Input buffer:", IOCTL_DI_RESET);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_RESET failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_RESET done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_ReadDiskId(void *id)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Read disc ID */
	inbuf[0] = IOCTL_DI_READID << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_READID (%x), Input buffer:", IOCTL_DI_READID);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_READID, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_READID failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("Output buffer:");
	ghexdump(outbuf, sizeof(dvddiskid));
	gprintf("IOCTL_DI_READID done\n");
#endif
	if (ret == 1) {
		memcpy(id, outbuf, sizeof(dvddiskid));
		return 0;
	}

	return -ret;
}

s32 WDVD_Seek(u64 offset)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Drive seek */
	inbuf[0] = IOCTL_DI_SEEK << 24;
	inbuf[1] = (u32)(offset >> 2);

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_SEEK (%x), Input buffer:", IOCTL_DI_SEEK);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_SEEK, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_SEEK failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_SEEK done\n");
#endif	
	return (ret == 1) ? 0 : -ret;

}

s32 WDVD_Offset(u64 offset) {
    //u32 *off = (u32 *)((void *)&offset);
	union { u64 off64; u32 off32[2]; } off;off.off64 = offset;
    s32 ret;

    memset(inbuf, 0, sizeof(inbuf));

    /* Set offset */
    inbuf[0] = IOCTL_DI_OFFSET << 24;
    inbuf[1] = (off.off32[0]) ? 1: 0;
    inbuf[2] = (off.off32[1] >> 2);

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_OFFSET (%x), Input buffer:", IOCTL_DI_OFFSET);
	ghexdump(inbuf, sizeof(inbuf));
#endif
    ret = IOS_Ioctl(di_fd, IOCTL_DI_OFFSET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_OFFSET failed: %d\n", ret);
#endif
        return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_OFFSET done\n");
#endif
    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopLaser(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Stop laser */
	inbuf[0] = IOCTL_DI_STOPLASER << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_STOPLASER (%x), Input buffer:", IOCTL_DI_STOPLASER);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPLASER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_STOPLASER failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_STOPLASER done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopMotor(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Stop motor */
	inbuf[0] = IOCTL_DI_STOPMOTOR << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_STOPMOTOR (%x), Input buffer:", IOCTL_DI_STOPMOTOR);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPMOTOR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_STOPMOTOR failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_STOPMOTOR done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_OpenPartition(u64 offset, void* Ticket, void* Certificate, unsigned int Cert_Len, void* Out)
{
	static ioctlv	Vectors[5]		__attribute__((aligned(0x20)));
	s32 ret;

	memset(inbuf, 0, sizeof inbuf);
	memset(outbuf, 0, sizeof outbuf);

	inbuf[0] = IOCTL_DI_OPENPART << 24;
	inbuf[1] = offset >> 2;

	Vectors[0].data		= inbuf;
	Vectors[0].len		= 0x20;
	Vectors[1].data		= (Ticket == NULL) ? 0 : Ticket;
	Vectors[1].len		= (Ticket == NULL) ? 0 : 0x2a4;
	Vectors[2].data		= (Certificate == NULL) ? 0 : Certificate;
	Vectors[2].len		= (Certificate == NULL) ? 0 : Cert_Len;
	Vectors[3].data		= Out;
	Vectors[3].len		= 0x49e4;
	Vectors[4].data		= outbuf;
	Vectors[4].len		= 0x20;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_OPENPART (%x), Input buffer:", IOCTL_DI_OPENPART);
	ghexdump(Vectors, 0x20);
#endif
	ret = IOS_Ioctlv(di_fd, IOCTL_DI_OPENPART, 3, 2, (ioctlv *)Vectors);
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_OPENPART failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_OPENPART done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_OpenPartitionSimple(u64 offset)
{
	u8 Tmd_Buffer[0x49e4 + 0x1C];
	return WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer);
}

s32 WDVD_ClosePartition(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Close partition */
	inbuf[0] = IOCTL_DI_CLOSEPART << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_CLOSEPART (%x), Input buffer:", IOCTL_DI_CLOSEPART);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_CLOSEPART, inbuf, sizeof(inbuf), NULL, 0);
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_CLOSEPART failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_CLOSEPART done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_UnencryptedRead(void *buf, u32 len, u64 offset)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Unencrypted read */
	inbuf[0] = IOCTL_DI_UNENCREAD << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_UNENCREAD (%x), Input buffer:", IOCTL_DI_UNENCREAD);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_UNENCREAD, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_UNENCREAD failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("Output buffer:");
	ghexdump(buf, len);
	gprintf("IOCTL_DI_UNENCREAD done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Read(void *buf, u32 len, u64 offset)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Disc read */
	inbuf[0] = IOCTL_DI_READ << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_READ (%x), Input buffer:", IOCTL_DI_READ);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_READ, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_READ failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	if (len <= 0xFF) {
		gprintf("Output buffer:");
		ghexdump(buf, len);
	}
	gprintf("IOCTL_DI_READ done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_WaitForDisc(void)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Wait for disc */
	inbuf[0] = IOCTL_DI_WAITCVRCLOSE << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_WAITCVRCLOSE (%x), Input buffer:", IOCTL_DI_WAITCVRCLOSE);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_WAITCVRCLOSE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_WAITCVRCLOSE failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("IOCTL_DI_WAITCVRCLOSE done\n");
#endif
	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_GetCoverStatus(u32 *status)
{
	s32 ret;

	memset(inbuf, 0, sizeof(inbuf));

	/* Get cover status */
	inbuf[0] = IOCTL_DI_GETCOVER << 24;

#ifdef DEBUG_WDVD
	gprintf("\nSending IOCTL_DI_GETCOVER (%x), Input buffer:", IOCTL_DI_GETCOVER);
	ghexdump(inbuf, sizeof(inbuf));
#endif
	ret = IOS_Ioctl(di_fd, IOCTL_DI_GETCOVER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) {
#ifdef DEBUG_WDVD
		gprintf("IOCTL_DI_GETCOVER failed: %d\n", ret);
#endif
		return ret;
	}
#ifdef DEBUG_WDVD
	gprintf("Output buffer:");
	ghexdump(outbuf, sizeof(outbuf));
	gprintf("IOCTL_DI_GETCOVER done\n");
#endif
	if (ret == 1) {
		/* Copy cover status */
		memcpy(status, outbuf, sizeof(u32));

		return 0;
	}

	return -ret;
}
