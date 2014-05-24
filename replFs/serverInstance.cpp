/*
 * serverInstance.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */
#include "serverInstance.h"
#include <sys/stat.h>

ServerInstance::ServerInstance() {

}

ServerInstance::ServerInstance(int port, std::string mount, int dropRate) {
  N = new Network(GROUP, port, dropRate, SERVER);
  this->mount = mount;
  this->fullPath = "";
  this->isOpened = false;
  this->numMissingBlocks = 0;
  this->numTotalBlocks = 0;
  this->transNum = 0;

  this->fd = 0;
  for (int i = 0; i < MAX_PENDING; i++) {
    pendingBlocks[i] = NULL;
  }
  if (mkdir(mount.c_str(), S_IRUSR | S_IWUSR) < 0) {
    INFO("dir already exist\n");
  }

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
    INFO("already opened on server, open again\n\n");
  }
  pb->printPacket();
  isOpened = true;
  fd = pb->fd;
  transNum = pb->transNum;
  OpenPkt* p = (OpenPkt*) pb;
  fullPath = mount + "/" + p->fileName;
  OpenAckPkt *ps = new OpenAckPkt(GUID, fd, 0, 0, true);
  N->send(ps);
  delete ps;
  delete pb;

}

void ServerInstance::processWriteBlock(PacketBase* pb) {
  DBG("========2==processing Writing block===========\n");
  pb->printPacket();
  if (pb->fd != fd || isOpened == false)
    return;
  WriteBlockPkt *p = (WriteBlockPkt*) pb;
  transNum = p->transNum;
  if (pendingBlocks[p->blockID]) {
    delete pendingBlocks[p->blockID];
  }
  pendingBlocks[p->blockID] = p;

  numTotalBlocks = 0;
  DBG(" print pending blocks:\n");
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i] != NULL) {
      numTotalBlocks++;
      DBG("%d ", i);
    }
  }
  DBG("total=%d\n", numTotalBlocks);
}

void ServerInstance::processCommitVoting(PacketBase* pb) {
  DBG("============333=========== processing Commit voting ============\n");
  pb->printPacket();
  if (pb->fd != fd || isOpened == false)
    return;
  if (transNum != pb->transNum) {
    ERROR("wrong transaction number!!\n");
    return;
  }
  CommitVotingPkt *p = (CommitVotingPkt *) pb;
  int missingBlocks[MAX_PENDING];
  int totalMissing = 0;
  int totalPending = p->totalPending;
  for (int i = 0; i < totalPending; i++) {
    if (pendingBlocks[i] == NULL || pendingBlocks[i]->transNum != p->transNum) {
      missingBlocks[totalMissing++] = i;
    }
  }
  if (totalMissing == 0) {
    CommitVotingSuccessPkt *ps = new CommitVotingSuccessPkt(GUID, fd, 0,
                                                            p->transNum);
    N->send(ps);
    delete ps;
  } else {
    CommitVotingResendPkt *ps = new CommitVotingResendPkt(GUID, fd, 0,
                                                          p->transNum,
                                                          totalMissing,
                                                          missingBlocks);
    N->send(ps);
    delete ps;
  }

  delete pb;

}

void ServerInstance::processCommitFinal(PacketBase* pb) {
  DBG("============444=========== processing Commit final ============   %s\n",
      fullPath.c_str());
  pb->printPacket();
  if (pb->fd != fd || isOpened == false)
    return;
  if (fullPath.c_str() == "")
    ERROR("the full path name is missing\n");

  FILE *f = fopen(fullPath.c_str(), "r+");
  if (f == NULL) {
    DBG(" ************* file not existed yet, creating a new file and overwrite the old  ************\n");
    f = fopen(fullPath.c_str(), "wb");
    if (f == NULL) {
      ERROR("error open file\n");
      delete pb;
      return;
    }
  }

  for (int i = 0; i < numTotalBlocks; i++) {
    if (pendingBlocks[i] != NULL
        && pendingBlocks[i]->transNum == pb->transNum) {

      fseek(f, pendingBlocks[i]->offset, SEEK_SET);
      fwrite(pendingBlocks[i]->payload, 1, pendingBlocks[i]->size, f);
    }
  }
  fclose(f);
  DBG("commited fd=%d, trans=%d, size=%d", fd, transNum, numTotalBlocks);
  CommitFinalReplyPkt *p = new CommitFinalReplyPkt(GUID, fd, 0, transNum, true);
  N->send(p);
  cleanup();
  delete pb;
  delete p;

}

void ServerInstance::processAbort(PacketBase* pb) {
  DBG("============555=========== processing abort ========   %s\n",
      fullPath.c_str());
  pb->printPacket();
  if (pb->fd != fd || isOpened == false)
    return;
  cleanup();
  delete pb;
}

void ServerInstance::processClose(PacketBase* pb) {
  DBG("============666=========== processing close ======== \n");
  pb->printPacket();
  if (pb->fd != fd || isOpened == false)
    return;
  CloseReplyPkt *p = new CloseReplyPkt(GUID, fd, 0, transNum, true);
  N->send(p);
  cleanup();
  transNum = 0;
  isOpened = false;
  fullPath = "";
  delete pb;
  delete p;
}

void ServerInstance::run() {
  PacketBase *p;
  while (1) {
    p = N->receive();
    if (p == NULL || p->nodeType == SERVER) {
      continue;
    } else {
      DBG("server received a new packet!\n");
      switch (p->type) {
        case OPEN:
          DBG("process open\n");
          processOpen(p);
          break;
        case WRITE_BLOCK:
          DBG("process write block\n");
          processWriteBlock(p);
          break;
        case COMMIT_VOTING:
          DBG("process voting\n");
          processCommitVoting(p);
          break;
        case COMMIT_FINAL:
          DBG("process commit final\n");
          processCommitFinal(p);
          break;
        case ABORT:
          processAbort(p);
          DBG("process abort\n");
          break;
        case CLOSE:
          DBG("process close\n");
          processClose(p);
          break;
        default:
          ERROR("unknow packet type\n");
      }
    }
  }
}

void ServerInstance::sweep(uint32_t transNum) {
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i] && pendingBlocks[i]->transNum < transNum) {
      INFO("obsolete block!\n");
      delete pendingBlocks[i];
      pendingBlocks[i] = NULL;
    }
  }
}

void ServerInstance::cleanup() {
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i]) {
      delete pendingBlocks[i];
      pendingBlocks[i] = NULL;
    }
  }
  numTotalBlocks = 0;
  numMissingBlocks = 0;
}
