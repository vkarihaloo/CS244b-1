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

ClientInstance *C;
/* ------------------------------------------------------------------ */
int InitReplFs(unsigned short portNum, int packetLoss, int numServers) {
  DBG("InitReplFs: Port number %d, packet loss %d percent, %d servers\n",
      portNum, packetLoss, numServers);

  /****************************************************/
  /* Initialize network access, local state, etc.     */
  /****************************************************/
  C = new ClientInstance(portNum, packetLoss, numServers);
  return (NormalReturn);
}

/* ------------------------------------------------------------------ */

int OpenFile(char * fileName) {
  int fd;
  DBG("OpenFile: Opening File '%s'\n", fileName);
  ASSERT(fileName);
  fd = C->OpenFile(fileName);

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

  DBG("WriteBlock: Writing FD=%d, Offset in the file =%d, Length in the buffer to write=%d\n",
      fd, byteOffset, blockSize);

  if ((bytesWritten = C->WriteBlock(fd, buffer, byteOffset, blockSize)) < 0) {
    perror("WriteBlock write");
    return (ErrorReturn);
  }
  return (bytesWritten);
}

/* ------------------------------------------------------------------ */

int Commit(int fd) {
  ASSERT(fd >= 0);

  DBG("Commit: FD=%d\n", fd);
  int ret;
  /****************************************************/
  /* Prepare to Commit Phase			    */
  /* - Check that all writes made it to the server(s) */
  /****************************************************/
  ret = C->CommitVoting(fd);
  if (ret == 1) //nothing to commit
    return NormalReturn;
  if (ret == ErrorReturn)
    return ErrorReturn;
  /****************/
  /* Commit Phase */
  /****************/
  ret = C->CommitFinal(fd);
  return ret;

}

/* ------------------------------------------------------------------ */

int Abort(int fd) {
  ASSERT(fd >= 0);

  DBG("Abort: FD=%d\n", fd);

  /*************************/
  /* Abort the transaction */
  /*************************/
  int ret = C->Abort(fd);

  if (ret < 0) {
    ERROR("Close");
    return (ErrorReturn);
  }

  return (NormalReturn);
}

/* ------------------------------------------------------------------ */

int CloseFile(int fd) {
  ASSERT(fd >= 0);

  DBG("Close: FD=%d\n", fd);

  /*****************************/
  /* Check for Commit or Abort */
  /*****************************/
  int ret = Commit(fd);
  if (ret == 0)
    ret = C->CloseFile(fd);

  if (ret < 0) {
    ERROR("Close");
    return (ErrorReturn);
  }
  return (NormalReturn);
}

/* ------------------------------------------------------------------ */
