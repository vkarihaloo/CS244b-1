/****************/
/* Song Han*/
/* Date May 17  */
/* CS 244B  */
/* Spring 2014  */
/****************/


#define DEBUG

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <debug.h>
#include <client.h>

/* ------------------------------------------------------------------ */

int InitReplFs(unsigned short portNum, int packetLoss, int numServers) {
#ifdef DEBUG
  printf("InitReplFs: Port number %d, packet loss %d percent, %d servers\n",
         portNum, packetLoss, numServers);
#endif

  /****************************************************/
  /* Initialize network access, local state, etc.     */
  /****************************************************/

  return (NormalReturn);
}

/* ------------------------------------------------------------------ */

int OpenFile(char * fileName) {
  int fd;

  ASSERT(fileName);

  DBG("OpenFile: Opening File '%s'\n", fileName);

  fd = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

  if (fd < 0)
    perror("OpenFile error");

  return (fd);
}

/* ------------------------------------------------------------------ */

int WriteBlock(int fd, char * buffer, int byteOffset, int blockSize) {
  //char strError[64];
  int bytesWritten;

  //TODO: return -1 for the following:
  ASSERT(fd >= 0);
  ASSERT(byteOffset >= 0);
  ASSERT(buffer);
  ASSERT(blockSize >= 0 && blockSize < MaxBlockLength);
  ASSERT(byteOffset + blockSize < MaxFileSize);

  DBG("WriteBlock: Writing FD=%d, Offset in the file =%d, Length in the buffer to write=%d\n", fd, byteOffset,
      blockSize);

  if (lseek(fd, byteOffset, SEEK_SET) < 0) {
    perror("WriteBlock Seek");
    return (ErrorReturn);
  }

  if ((bytesWritten = write(fd, buffer, blockSize)) < 0) {
    perror("WriteBlock write");
    return (ErrorReturn);
  }
  return (bytesWritten);
}

/* ------------------------------------------------------------------ */

int Commit(int fd) {
  ASSERT(fd >= 0);

#ifdef DEBUG
  printf("Commit: FD=%d\n", fd);
#endif

  /****************************************************/
  /* Prepare to Commit Phase			    */
  /* - Check that all writes made it to the server(s) */
  /****************************************************/

  /****************/
  /* Commit Phase */
  /****************/

  return (NormalReturn);

}

/* ------------------------------------------------------------------ */

int Abort(int fd) {
  ASSERT(fd >= 0);

#ifdef DEBUG
  printf("Abort: FD=%d\n", fd);
#endif

  /*************************/
  /* Abort the transaction */
  /*************************/

  return (NormalReturn);
}

/* ------------------------------------------------------------------ */

int CloseFile(int fd) {

  ASSERT(fd >= 0);

  DBG("Close: FD=%d\n", fd);

  /*****************************/
  /* Check for Commit or Abort */
  /*****************************/

  if (close(fd) < 0) {
    perror("Close");
    return (ErrorReturn);
  }

  return (NormalReturn);
}

/* ------------------------------------------------------------------ */

