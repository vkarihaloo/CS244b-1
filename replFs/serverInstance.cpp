/*
 * serverInstance.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */
#include "serverInstance.h"

ServerInstance::ServerInstance() {

}

ServerInstance::ServerInstance(int port, std::string mount, int dropRate) {
  N = new Network(GROUP, port, dropRate);
  this->mount = mount;
  this->fullPath = "";
  this->isOpened = false;
  this->numPendingBlocks = 0;
  this->numMissingBlocks = 0;
  this->seqNum = 0;

}

ServerInstance::~ServerInstance() {
  delete N;
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i] != NULL)
      delete pendingBlocks[i];
  }
}

void ServerInstance::processOpen(PacketBase* pb) {
  if (isOpened) {
    INFO("already opened on server!!");
    return;
  }
  pb->printPacket();
  isOpened = true;
  OpenPkt* p = (OpenPkt*) pb;
  fd = pb->fd;
  // record down the name
  fullPath = mount + "/" + p->fileName;
  OpenAckPkt *ps = new OpenAckPkt(GUID, fd, 0, 0, true);
  N->send(ps);

}

void ServerInstance::processWriteBlock(PacketBase* pb) {
  pb->printPacket();
  if (pb->fd != fd)
    return;
  WriteBlockPkt *p = (WriteBlockPkt*) pb;
  pendingBlocks[p->blockID] = p;
  numPendingBlocks++;

}

void ServerInstance::processCommitVoting(PacketBase* pb) {
  pb->printPacket();
}

void ServerInstance::processCommitFinal(PacketBase* pb) {
  pb->printPacket();
}

void ServerInstance::processAbort(PacketBase* pb) {
  pb->printPacket();
}

void ServerInstance::processClose(PacketBase* pb) {
  pb->printPacket();
}

void ServerInstance::run() {
  PacketBase *p;
  while (1) {
    p = N->receive();
    if (p == NULL || p->nodeType == SERVER) {
      continue;
    } else {
      DBG("server received a packet!\n");
      switch (p->type) {
        case OPEN:
          processOpen(p);
          break;
        case WRITE_BLOCK:
          processWriteBlock(p);
          break;
        case COMMIT_VOTING:
          processCommitVoting(p);
          break;
        case COMMIT_FINAL:
          processCommitFinal(p);
          break;
        case ABORT:
          processAbort(p);
          break;
        case CLOSE:
          processClose(p);
          break;
        default:
          ERROR("unknow packet type\n");
      }
    }
  }
}


