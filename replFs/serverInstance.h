/*
 * serverInstance.h
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#ifndef SERVERINSTANCE_H_
#define SERVERINSTANCE_H_

#include "networkInstance.h"

class ServerInstance {
 private:
  int fd;
  std::string fullPath;
  std::string mount;
  uint32_t GUID;
  uint32_t seqNum;
  int numPendingBlocks;
  int numMissingBlocks;
  bool isOpened;
  std::string mount;

  std::map<int, int>::iterator iterSeqNum;  //mapping from machine id to that machine's seq number
  WriteBlockPkt* pendingBlocks[MAX_PENDING];  //mapping from block id to packet that contains the payload

  Network *N;

  bool outOfOrder(PacketBase *p);

  void  processOpen(PacketBase *p);
  void  processWriteBlock(PacketBase *p);
  void  processCommitVoting(PacketBase *p);
  void  processCommitFinal(PacketBase *p);
  void  processAbort(PacketBase *p);
  void  processClose(PacketBase *p);

 public:
  ServerInstance();
  ServerInstance(int port, int mount, int dropRate);
  virtual ~ServerInstance();
  void run();
};
#endif /* SERVERINSTANCE_H_ */

