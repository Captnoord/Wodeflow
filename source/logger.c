// we are really crude and atm I don't really care... work bitch

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "logger.h"

void log_printf( const char *str, ... )
{
//	if (log_fd == NULL) return;
	FILE * log_fd;
	log_fd = fopen("sd:/log.txt", "a");
	if (!log_fd)
		log_fd = fopen("sd:/log.txt", "a");
	
	va_list ap;
	va_start(ap,str);
	
	vfprintf(log_fd, str, ap );
	
	va_end(ap);
	fflush(log_fd);
	fclose(log_fd);
}

/**
 * function to write a buffer to a file handle formatted into a hex edit view.
 * @note function originating from 'Burlex' his utility framework so credits for this function goes to him.
 */
static void print_hexview(FILE *dstFile, const char *pSource, unsigned int sourceLength)
{
	fputs("--------|------------------------------------------------|----------------|\n", dstFile);
	fputs(" offset |00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\n", dstFile);
	fputs("--------|------------------------------------------------|----------------|\n", dstFile);

	size_t i = 0;
	size_t c = 0;
	size_t start;
	size_t written;
	unsigned char byte;
	const unsigned char *pData = (const unsigned char *)pSource;

	for( ; i < sourceLength; )
	{
		start = i;
		fprintf(dstFile,"%08X|", i);
		for( c = 0; c < 16 && i < sourceLength; )		// write 16 bytes per line
		{
			fprintf(dstFile, "%02X ", (int)pData[i]);
			++i; ++c;
		}

		written = c;
		for( ; c < 16; ++c )							// finish off any incomplete bytes
			fputs("   ", dstFile);

		// write the text part
		fputc('|', dstFile);
		for( c = 0; c < written; ++c )
		{
			byte = pData[start + c];
			if( isprint((int)byte) )
				fputc((char)byte, dstFile);
			else
				fputc('.', dstFile);
		}

		for( ; c < 16; ++c )
			fputc(' ', dstFile);

		fputs("|\n", dstFile);
	}

	fputs("---------------------------------------------------------------------------\n", dstFile);
}

void log_hex( const char *str, int len)
{
	FILE * log_fd;
	log_fd = fopen("sd:/log.txt", "a");
	fputs("\n", log_fd);
	print_hexview(log_fd, str, len);
	
	fclose(log_fd);
}
