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

  uint32_t transNum;
  int numMissingBlocks;
  int numTotalBlocks;
  bool isOpened;

  WriteBlockPkt* pendingBlocks[MAX_PENDING];  //mapping from block id to packet that contains the payload

  Network *N;

  void processOpen(PacketBase *p);
  void processWriteBlock(PacketBase *p);
  void processCommitVoting(PacketBase *p);
  void processCommitFinal(PacketBase *p);
  void processAbort(PacketBase *p);
  void processClose(PacketBase *p);
  void sweep(uint32_t transNum);
  void cleanup();
 public:
  ServerInstance();
  ServerInstance(int port, std::string mount, int dropRate);
  virtual ~ServerInstance();
  void run();
};
#endif /* SERVERINSTANCE_H_ */

