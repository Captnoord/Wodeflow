#define ALIGN(x) (((x) + 3) & ~3)
#define ALIGN32(x) (((x) + 31) & ~31)

#define SMART_FREE(P)		{if(!!P)P.release();}
#define SAFE_FREE(P)		{if(P != NULL){free(P);P = NULL;}}
#define MEM2_SAFE_FREE(P)	{if(P){MEM2_free(P);P = NULL;}}
#define SAFE_DELETE(P)		{if(P != NULL){delete P;P = NULL;}}
#define SAFE_CLOSE(P)		{if(P != NULL){fclose(P);P = NULL; }}