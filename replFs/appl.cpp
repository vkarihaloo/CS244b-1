/************************/
/* Your Name: Wei Shi   */
/* Date: May 9, 2013    */
/* CS 244B              */
/* Spring 2013          */
/************************/

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <client.h>
#include <appl.h>

#define FS_PORT 44024

static void appl0();
static void appl3();
static void appl7();
static void appl8();
static void appl9();
static void appl13();

static int openFile(char *file) {
  int fd = OpenFile(file);
  if (fd < 0) {
    printf("OpenFile(%s): failed (%d)\n", file, fd);
    exit(-1);
  }
  return fd;
}

static int commit(int fd) {
  int result = Commit(fd);
  if (result < 0) {
    printf("Commit(%d): failed (%d)\n", fd, result);
    exit(-1);
  }
  return fd;
}

static int closeFile(int fd) {
  int result = CloseFile(fd);
  if (result < 0) {
    printf("CloseFile(%d): failed (%d)\n", fd, result);
    exit(-1);
  }
  return fd;
}

int main() {
  if (InitReplFs( FS_PORT, 20, 5) < 0) {
    fprintf( stderr, "Error initializing the system\n");
    return (ErrorExit);
  }
  app7();
  return (NormalExit);
}

static void appl0() {
  int fd;
  int loopCnt;
  int byteOffset = 0;
  char strData[MaxBlockLength];

  char fileName[32] = "writeTest.txt";

  for (loopCnt = 0; loopCnt < 8; loopCnt++) {
    fd = openFile(fileName);
    sprintf(strData, "%08X", loopCnt);

    if (WriteBlock(fd, strData, byteOffset, strlen(strData)) < 0) {
      printf("Error writing to file %s [LoopCnt=%d]\n", fileName, loopCnt);
    }
    byteOffset += strlen(strData);

    commit(fd);
    closeFile(fd);

  }

}

static void appl8() {
  int fd;
  int retVal;
  int i;
  char commitStrBuf[512];
  char *filename = (char *) "file8";
  for (i = 0; i < 512; i++)
    commitStrBuf[i] = '1';

  fd = openFile(filename);

  // write first transaction starting at offset 512
  for (i = 0; i < 50; i++) {
    retVal = WriteBlock(fd, commitStrBuf, 512 + i * 512, 512);
  }

  retVal = commit(fd);
  retVal = closeFile(fd);

  for (i = 0; i < 512; i++)
    commitStrBuf[i] = '2';

  fd = openFile(filename);

  // write second transaction starting at offset 0
  retVal = WriteBlock(fd, commitStrBuf, 0, 512);

  retVal = commit(fd);
  retVal = closeFile(fd);

  for (i = 0; i < 512; i++)
    commitStrBuf[i] = '3';

  fd = openFile(filename);

  // write third transaction starting at offset 50*512
  for (i = 0; i < 100; i++)
    retVal = WriteBlock(fd, commitStrBuf, 50 * 512 + i * 512, 512);

  retVal = commit(fd);
  retVal = closeFile(fd);
  return retVal;
}

/* ------------------------------------------------------------------ */

static void appl3() {

  //  simple case - just abort a single update on a new file
  //  the file  should not be in the mount directory at the end given
  //  it was not existing to start with. Script should check that

  int fd;
  int retVal;

  fd = openFile("file3");
  retVal = WriteBlock(fd, "abcdefghijkl", 0, 12);
  Abort(fd);
  retVal = closeFile(fd);  // should abort and remove the file
}

//Used for packetloss check
static void appl13() {
  // simple case - just commit a single write update.

  int fd;
  int retVal;

  fd = openFile("file13");
  retVal = WriteBlock(fd, "abcdefghijkl", 0, 12);
  retVal = commit(fd);
  retVal = closeFile(fd);
}

static void appl9() {
  // multiple aborts and then commits of the same file. As a consequence,
  // this also checks that
  // when a file exists in the mount directory, they should openFile it
  // and not create a new one.
  // also try out max. number of write updates to stress
  // No X, Y or Z should be in the file. The file should have
  // only '0' s and '%' at offset  100-200 and 1000000 - 1000300

  int fd;
  int retVal;
  int i;
  char commitStrBuf[512];

  for (i = 0; i < 512; i++)
    commitStrBuf[i] = '0';

  fd = openFile("file9");

  // zero out a max. size file with max. number of writes
  // was 2048, now is 127
  for (i = 0; i < 127; i++)
    retVal = WriteBlock(fd, commitStrBuf, i * 512, 512);

  retVal = commit(fd);
  retVal = closeFile(fd);

  // now try bunch of aborts on the file.

  fd = openFile("file9");

  for (i = 0; i < 29; i++)
    retVal = WriteBlock(fd, "XXXXXXX", i * 200, 7);

  Abort(fd);
  retVal = closeFile(fd);

  fd = openFile("file9");

  for (i = 0; i < 59; i++)
    retVal = WriteBlock(fd, "YYYYYYY", 8888 + i * 200, 7);

  Abort(fd);
  retVal = closeFile(fd);

  fd = openFile("file9");

  for (i = 0; i < 109; i++)
    retVal = WriteBlock(fd, "ZZZZZ", 33333 + i * 100, 5);

  Abort(fd);
  retVal = closeFile(fd);

  DBG("Done aborting.\n");

  fd = openFile("file9");
  for (i = 0; i < 10; i++)
    retVal = WriteBlock(fd, "%%%%%%%%%%", 100 + i * 10, 10);
  retVal = commit(fd);
  retVal = closeFile(fd);

  fd = openFile("file9");
  for (i = 0; i < 60; i++)
    retVal = WriteBlock(fd, "%%%%%", 1000000 + i * 5, 5);
  retVal = commit(fd);
  retVal = closeFile(fd);

}

static void appl7() {
  // try out commit - abort - commit sequence
  // No XXX should be in the file

  int fd;
  int retVal;
  int i;
  char * commitStr = "deadbeef";
  char * abortStr = "XXX";
  char * commitStr2 = "1234567";

  int commitStrLength = strlen(commitStr);
  int abortStrLength = strlen(abortStr);
  int commitStr2Length = strlen(commitStr2);

  fd = openFile("file7");
  for (i = 0; i < 29; i++)
    retVal = WriteBlock(fd, commitStr, i * commitStrLength, commitStrLength);
  retVal = commit(fd);

  for (i = 0; i < 7; i++)
    retVal = WriteBlock(fd, abortStr, i * abortStrLength, abortStrLength);
  Abort(fd);

  for (i = 29; i < 50; i++)
    retVal = WriteBlock(fd, commitStr2, i * commitStr2Length, commitStr2Length);

  retVal = commit(fd);
  retVal = closeFile(fd);

}
