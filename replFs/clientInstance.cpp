/*
 * ClientInstance.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#include "clientInstance.h"
#include <assert.h>
ClientInstance::ClientInstance() {

}

ClientInstance::ClientInstance(int port, int dropRate, int numServers) {
  this->fileName = "";
  this->numServers = numServers;
  this->fd = 0;
  srand(time(NULL));
  this->GUID = rand();
  this->seqNum = 0;
  this->numPendingBlocks = 0;
  N = new Network(GROUP, port, dropRate);
}

ClientInstance::~ClientInstance() {
  delete N;
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i] != NULL)
      delete pendingBlocks[i];
  }
}

int ClientInstance::OpenFile(char* fileName) {
  fd++; seqNum++
  OpenPkt *p = new OpenPkt(GUID, fd, )
}

int ClientInstance::WriteBlock(int fd, char* strData, int byteOffset,
                               int blockSize) {
}

int ClientInstance::Abort(int fd) {
}

int ClientInstance::CloseFile(int fd) {
}

int ClientInstance::CommitVoting(int fd) {
}

int ClientInstance::CommitFinal(int fd) {
}
