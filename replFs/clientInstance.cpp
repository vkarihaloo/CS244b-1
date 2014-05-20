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

ClientInstance::ClientInstance(unsigned short port, int dropRate,
                               int numServers) {
  this->fileName = "";
  this->numServers = numServers;
  this->fd = 0;
  this->isOpened = false;
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
  INFO(" the newly opened fd is %d", fd);
  OpenPkt *p = new OpenPkt((uint32_t) GUID, fd, (uint32_t) 0, (uint32_t) 0,
                           fileName);
  N->send(p);

  DBG("send a packet!");
  return fd;
}

int ClientInstance::WriteBlock(int fd_, char* strData, int byteOffset,
                               int blockSize) {
  if (fd != fd_)
    return -1;
}

int ClientInstance::CommitVoting(int fd_) {
  if (fd != fd_)
    return -1;
}

int ClientInstance::CommitFinal(int fd_) {
  if (fd != fd_)
    return -1;
}

int ClientInstance::Abort(int fd_) {
  if (fd != fd_)
    return -1;
}

int ClientInstance::CloseFile(int fd_) {
  if (fd != fd_)
    return -1;
}

bool ClientInstance::isTimeOut(timeval oldTime, long timeOut) {
  struct timeval curTime;
  gettimeofday(&curTime, NULL);
  if ((curTime.tv_sec - oldTime.tv_sec) * 1000
      + (curTime.tv_usec - oldTime.tv_usec) / 1000 > timeOut)
    return true;
  else
    return false;
}

int ClientInstance::collectResponse(PacketBase *ps) {

  struct timeval beginTime;
  struct timeval againTime;
  gettimeofday(&beginTime, NULL);
  gettimeofday(&againTime, NULL);
  std::set<uint32_t> setResponses;

  while (isTimeOut(beginTime, COLLECT_RESPONSE_TIME_OUT) == false) {
    if (isTimeOut(againTime, COLLECT_RESPONSE_TIME_OUT)) {
      N->send(ps);  //send again
      gettimeofday(&againTime, NULL);
    }
    PacketBase *pr = N->receive();
    if (pr == NULL || mismatch(ps, pr) == false)
      continue;
    switch (pr->type) {
      case OPEN_ACK:
        OpenAckPkt* p = (OpenAckPkt*)pr;
        if (p->status == true){
          setResponses.insert(p->GUID);
          if (setResponses.size()>=numServers){
            isOpened = true;
            return fd;
          }


        }

        break;
      case COMMIT_VOTING_SUCCESS:

        break;
      case COMMIT_VOTING_RESEND:

        break;
      case COMMIT_FINAL_REPLY:

        break;
      default:
        ERROR("wrong reply from server to client\n");
        break;

    }

  }

  return 0;

}

bool ClientInstance::mismatch(PacketBase *ps, PacketBase *pr) {
  if (ps == NULL || pr == NULL) {
    ERROR("NULL packet pairs!");
    return 0;
  }
  switch (ps->type) {
    case OPEN:
      if (pr->type == OPEN_ACK)
        return true;
      else
        return false;
    case COMMIT_VOTING:
    case CLOSE:
      if (pr->type == COMMIT_VOTING_SUCCESS || pr->type == COMMIT_VOTING_RESEND)
        return true;
      else
        return false;
    case COMMIT_FINAL:
      if (pr->type == COMMIT_FINAL_REPLY)
        return true;
      else
        return false;
    case ABORT:
    case WRITE_BLOCK:
      ERROR("abort or write_block shoudn't wait for reply !!! \n");
    default:
      ERROR("send type error!!! not a clientIntance send type \n");
      return false;

  }

}
