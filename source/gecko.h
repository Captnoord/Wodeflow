

#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define NO_DEBUG
#ifndef NO_DEBUG
	//use this just like printf();
    #define gprintf _gprintf
	void _gprintf(const char *str, ...);
	void gsenddata(const u8 *data, int length, const char *filename);
	void ghexdump(void *d, int len);
	bool InitGecko();
#else
	#define gprintf(str,...)    do {} while(0)
	#define gsenddata(...)      do {} while(0)
    #define ghexdump(...)       do {} while(0)
	#define InitGecko()         false
#endif /* NO_DEBUG */

#ifdef __cplusplus
}
#endif

#endif
