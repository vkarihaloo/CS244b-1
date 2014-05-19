/*
 * clientInstance.h
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#ifndef CLIENTINSTANCE_H_
#define CLIENTINSTANCE_H_
#include "networkInstance.h"
#include <map>
#include <string>

class ClientInstance {
 private:
  int fd;
  std::string fileName;
  int numServers;
  uint32_t GUID;
  uint32_t seqNum;
  int numPendingBlocks;
  std::map<int, int> mapSeqNum;  //mapping from machine id to that machine's seq number
  std::map<int, int>::iterator iterSeqNum;  //mapping from machine id to that machine's seq number
  WriteBlockPkt* pendingBlocks[MAX_PENDING];  //mapping from block id to packet that contains the payload
  Network *N;

 public:
  ClientInstance();
  ClientInstance(int port, int dropRate, int numServers);
  virtual ~ClientInstance();
  int OpenFile(char* fileName);
  int WriteBlock(int fd, char * strData, int byteOffset, int blockSize);
  int Abort(int fd);
  int CloseFile(int fd);

  int CommitVoting(int fd);
  int CommitFinal(int fd);

};
#endif /* CLIENTINSTANCE_H_ */

