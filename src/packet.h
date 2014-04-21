#ifndef PACKET_H
#define PACKET_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
//#include "mazewar.h"

#define HEART_BEAT 0
#define NAME_REQUEST 1
#define NAME_REPLY 2
#define GAME_EXIT 3

#define MAX_RATS   8
#define MAX_NAME 20

class PacketBase {
 public:
  uint8_t type;
  uint8_t userId;
  uint16_t checkSum;
  uint32_t seqNum;
  PacketBase() {
  }
  PacketBase(uint8_t type_, uint8_t userId_, uint16_t checkSum_,
             uint32_t seqNum_)
      : type(type_),
        userId(userId_),
        checkSum(0),
        seqNum((seqNum_)) {
  }
  void printPacket(bool isSend);
  uint16_t cksum(const void *_data, int len);

}__attribute__ ((packed));

class HeartBeatPkt : public PacketBase {
 public:
  int16_t ratX;
  int16_t ratY;
  int16_t ratD;
  int16_t scoreBase;
  int16_t misX;
  int16_t misY;
  int16_t hitCount[MAX_RATS];
  HeartBeatPkt() {
  }
  HeartBeatPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
               int16_t ratX_, int16_t ratY_, int16_t ratD_, int16_t scoreBase_,
               int16_t misX_, int16_t misY_, int16_t *hitCount_)

      : PacketBase(HEART_BEAT, userId_, htons(checkSum_), htonl(seqNum_)),
        ratX(htons(ratX_)),
        ratY(htons(ratY_)),
        ratD(htons(ratD_)),
        scoreBase(htons(scoreBase_)),
        misX(htons(misX_)),
        misY(htons(misY_)) {
    for (int i = 0; i < MAX_RATS; i++) {
      hitCount[i] = htons(hitCount_[i]);
    }
    checkSum = cksum(this, sizeof(HeartBeatPkt));
//    printf("the assembled checksum = %d\n", checkSum);
  }
  void printPacket(bool isSend);
  bool checkSumCorrect() {
    return cksum(this, sizeof(HeartBeatPkt)) == 0xffff;
  }
}__attribute__ ((packed));

class NameRequestPkt : public PacketBase {
 public:
  uint8_t targetUserId;
  char name[MAX_NAME];
  NameRequestPkt() {
  }
  NameRequestPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
                 uint8_t targetUserId_, const char *name_)
      : PacketBase(NAME_REQUEST, userId_, htons(checkSum_), htonl(seqNum_)),
        targetUserId(targetUserId_) {
    int i;
    for (i = 0; i < strlen(name_) && i < MAX_NAME - 1; i++) {
      name[i] = name_[i];
    }
    name[i] = '\0';
    checkSum = cksum(this, sizeof(NameRequestPkt));
  }
  bool checkSumCorrect() {
    return cksum(this, sizeof(NameRequestPkt)) == 0xffff;
  }

}__attribute__ ((packed));

class NameReplyPkt : public PacketBase {
 public:
  char name[MAX_NAME];
  NameReplyPkt() {
  }
  NameReplyPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
               const char *name_)
      : PacketBase(NAME_REPLY, userId_, htons(checkSum_), htonl(seqNum_)) {
    int i;
    for (i = 0; i < strlen(name_) && i < MAX_NAME - 1; i++) {
      name[i] = name_[i];
    }
    name[i] = '\0';
    checkSum = cksum(this, sizeof(NameReplyPkt));
  }
  bool checkSumCorrect() {
    return cksum(this, sizeof(NameReplyPkt)) == 0xffff;
  }

}__attribute__ ((packed));

class GameExitPkt : public PacketBase {
 public:
  int16_t ratX;
  int16_t ratY;
  int16_t ratD;
  int16_t scoreBase;
  int16_t misX;
  int16_t misY;
  int16_t hitCount[MAX_RATS];
  GameExitPkt() {
  }
  GameExitPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
              int16_t ratX_, int16_t ratY_, int16_t ratD_, int16_t scoreBase_,
              int16_t misX_, int16_t misY_, int16_t *hitCount_)
      : PacketBase(GAME_EXIT, userId_, htons(checkSum_), htonl(seqNum_)),
        ratX(htons(ratX_)),
        ratY(htons(ratY_)),
        ratD(htons(ratD_)),
        scoreBase(htons(scoreBase_)),
        misX(htons(misX_)),
        misY(htons(misY_)) {
    for (int i = 0; i < MAX_RATS; i++) {
      hitCount[i] = htons(hitCount_[i]);
    }
    checkSum = cksum(this, sizeof(GameExitPkt));
//    printf("the assembled checksum = %d\n", checkSum);
  }
  bool checkSumCorrect() {
    return cksum(this, sizeof(GameExitPkt)) == 0xffff;
  }
}__attribute__ ((packed));

#endif
