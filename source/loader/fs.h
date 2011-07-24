#ifndef _FAT_H_
#define _FAT_H_

#define APPDATA_DIR		"wodeflow"
#define CFG_FILENAME	"wodeflow.ini"
#define LANG_FILENAME	"language.ini"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
bool Fat_Mount(void);
void Fat_Unmount(void);
bool Fat_SDAvailable(void);
bool Fat_USBAvailable(void);

extern int   fs_sd_mount;
extern int	 fs_usb_mount;

extern bool g_sdOK;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
