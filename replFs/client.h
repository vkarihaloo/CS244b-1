/****************/
/* Song Han*/
/* Date	May 17	*/
/* CS 244B	*/
/* Spring 2014	*/
/****************/
#include "clientInstance.h"


enum {
  NormalReturn = 0,
  ErrorReturn = -1,
};

/* ------------------------------------------------------------------ */

#ifdef ASSERT_DEBUG
#define ASSERT(ASSERTION) \
 { assert(ASSERTION); }
#else
#define ASSERT(ASSERTION) \
{ }
#endif

/* ------------------------------------------------------------------ */

/********************/
/* Client Functions */
/********************/
#ifdef __cplusplus
extern "C" {
#endif

extern int InitReplFs(unsigned short portNum, int packetLoss, int numServers);
extern int OpenFile(char * strFileName);
extern int WriteBlock(int fd, char * strData, int byteOffset, int blockSize);
extern int Commit(int fd);
extern int Abort(int fd);
extern int CloseFile(int fd);

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------ */

