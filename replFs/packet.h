/*
 * packet.h
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include "debug.h"

#define MAX_FILE_NAME 128
#define MAX_PAY_LOAD 512
#define MAX_PENDING 128
#define MAX_FILE_SIZE (1<<20)

enum PacketType {
  OPEN,
  OPEN_ACK,
  WRITE_BLOCK,
  COMMIT_VOTING,
  COMMIT_VOTING_SUCCESS,
  COMMIT_VOTING_RESEND,
  COMMIT_FINAL,
  COMMIT_FINAL_REPLY,
  ABORT,
  CLOSE
};

enum NodeType {
  CLIENT,
  SERVER
};

class PacketBase {
 public:
  uint8_t type;
  uint8_t nodeType;
  uint16_t checkSum;
  uint32_t GUID;
  int fd;
  uint32_t seqNum;
  uint32_t transNum;

  PacketBase(uint8_t type, uint8_t nodeType, uint32_t GUID, int fd,
             uint32_t seqNum, uint32_t transNum);

  PacketBase() {
  }
  virtual ~PacketBase() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
                                                        //const, because it can't change members
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
  uint16_t cksum(const void *_data, int len);
  bool checkSumCorrect();

}
;

class OpenPkt : public PacketBase {
 public:
  char fileName[MAX_FILE_NAME];

  OpenPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
          char * fileName);
  OpenPkt() {
  }
//  virtual ~OpenPkt() {
//  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class OpenAckPkt : public PacketBase {
 public:
  bool status;

  OpenAckPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
             bool status);
  OpenAckPkt() {
  }
  virtual ~OpenAckPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class WriteBlockPkt : public PacketBase {
 public:
  int blockID;
  int offset;
  int size;
  uint8_t payload[MAX_PAY_LOAD];

  WriteBlockPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
                int blockID, int offset, int size, uint8_t * payload);
  WriteBlockPkt() {
  }
  virtual ~WriteBlockPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class CommitVotingPkt : public PacketBase {
 public:
  int totalPending;

  CommitVotingPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
                  int totalPending);
  CommitVotingPkt() {
  }
  virtual ~CommitVotingPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class CommitVotingSuccessPkt : public PacketBase {
 public:
  CommitVotingSuccessPkt(uint32_t GUID, int fd, uint32_t seqNum,
                         uint32_t transNum);
  CommitVotingSuccessPkt() {
  }
  virtual ~CommitVotingSuccessPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class CommitVotingResendPkt : public PacketBase {
 public:
  int totalMissing;
  std::vector<int> vectorMissingID;
//TODO:
  CommitVotingResendPkt(uint32_t GUID, int fd, uint32_t seqNum,
                        uint32_t transNum, int totalMissing, int *MissingIDs);
  CommitVotingResendPkt() {
  }
  virtual ~CommitVotingResendPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class CommitFinalPkt : public PacketBase {
 public:
  CommitFinalPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum);
  CommitFinalPkt() {
  }
  virtual ~CommitFinalPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class CommitFinalReplyPkt : public PacketBase {
 public:
  bool status;

  CommitFinalReplyPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
                      bool status);
  CommitFinalReplyPkt() {
  }
  virtual ~CommitFinalReplyPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class AbortPkt : public PacketBase {
 public:

  AbortPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum);
  AbortPkt() {
  }
  virtual ~AbortPkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

class ClosePkt : public PacketBase {
 public:
  int totalPending;

  ClosePkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
           int totalPending);
  ClosePkt() {
  }
  virtual ~ClosePkt() {
  }
  virtual void serialize(std::ostream& stream) const;   //upload it to stream
  virtual void deserialize(std::istream& stream);  //download from the stream and assemble the struct
  virtual void printPacket();
};

//overloaded << and >> for serialization and de-serialization. Since all serialization methods are
//virtual, this supports polymorphism and dynamic dispatch
std::istream& operator>>(std::istream &stream, PacketBase *packet);

std::ostream& operator<<(std::ostream &stream, const PacketBase *packet);

#endif /* PACKET_H_ */

