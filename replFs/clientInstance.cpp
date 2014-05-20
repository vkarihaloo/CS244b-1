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

ClientInstance::ClientInstance(unsigned short port, int dropRate, int numServers) {
  this->fileName = "";
  this->numServers = numServers;
  this->fd = 0;
  srand(time(NULL));
  this->GUID = rand();
  this->seqNum = 0;
  this->numPendingBlocks = 0;
  N = new Network(GROUP, port, dropRate);
  INFO("init clientIntance done\n");
}

ClientInstance::~ClientInstance() {
  delete N;
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingBlocks[i] != NULL)
      delete pendingBlocks[i];
  }
}

int ClientInstance::OpenFile(char* fileName) {
  fd++;
  OpenPkt *p = new OpenPkt((uint32_t)GUID, fd, (uint32_t)0, (uint32_t)0, fileName);
  N->send(p);
  N->send(p);
  N->send(p);
  DBG("send a packet!");
  return 0;
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
