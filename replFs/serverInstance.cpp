/*
 * serverInstance.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */
#include "serverInstance.h"

int main(int argc, char **argv) {

  return 0;
}

ServerInstance::ServerInstance() {

}

ServerInstance::ServerInstance(int port, int mount, int dropRate) {
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
}

void ServerInstance::processWriteBlock(PacketBase* p) {
}

void ServerInstance::processCommitVoting(PacketBase* p) {
}

void ServerInstance::processCommitFinal(PacketBase* p) {
}

void ServerInstance::processAbort(PacketBase* p) {
}

void ServerInstance::processClose(PacketBase* p) {
}

void ServerInstance::run() {
  PacketBase *p;
  while (1) {
    p = N->receive();
    if (p == NULL) {
      continue;
    } else {
      if (p->nodeType == SERVER || outOfOrder(p)) {
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

bool ServerInstance::outOfOrder(PacketBase* p) {
  if (mapSeqNum.count(p->GUID) != 0) {
    if (mapSeqNum[p->GUID] > p->seqNum) {
      return 1;
    } else {
      return 0;
    }
  } else {
    mapSeqNum[p->GUID] = p->seqNum;
    return 0;
  }
}

