#ifndef _LOGGER_H_
#define _LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

void log_printf( const char *str, ... );
void log_hex( const char *str, int len);


#ifdef __cplusplus
}
#endif

#endif