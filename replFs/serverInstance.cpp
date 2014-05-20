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

void ServerInstance::processOpen(PacketBase* p) {
  p->printPacket();
  isOpened = true;
}

void ServerInstance::processWriteBlock(PacketBase* p) {
  p->printPacket();
  if (p->fd != fd)

}

void ServerInstance::processCommitVoting(PacketBase* p) {
  p->printPacket();
}

void ServerInstance::processCommitFinal(PacketBase* p) {
  p->printPacket();
}

void ServerInstance::processAbort(PacketBase* p) {
  p->printPacket();
}

void ServerInstance::processClose(PacketBase* p) {
  p->printPacket();
}

void ServerInstance::run() {
  PacketBase *p;
  while (1) {
    p = N->receive();
    if (p == NULL) {
      continue;
    } else {
      DBG("server received a packet!\n");
      if (p->nodeType == SERVER ) {
        continue;
      } else {
        switch (p->type){
          case OPEN:
            processOpen(p);
          case WRITE_BLOCK:
            processWriteBlock(p);
          case COMMIT_VOTING:
            processCommitVoting(p);
          case COMMIT_FINAL:
            processCommitFinal(p);
          case ABORT:
            processAbort(p);
          case CLOSE:
            processClose(p);
        }
      }
    }
  }

}


