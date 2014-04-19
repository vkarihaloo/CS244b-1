#ifndef PACKET_H
#define PACKET_H

#include <string.h>
#include "mazewar.h"

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
        checkSum(htons(checkSum_)),
        seqNum(htonl(seqNum_)) {
 printf("size of base is %d\n", sizeof(PacketBase));
  }
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

      : PacketBase(HEART_BEAT, userId_, checkSum_, seqNum_),
        ratX(htons(ratX_)),
        ratY(htons(ratY_)),
        ratD(htons(ratD_)),
        scoreBase(htons(scoreBase_)),
        misX(htons(misX_)),
        misY(htons(misY_)) {
    for (int i = 0; i < MAX_RATS; i++) {
      hitCount[i] = htons(hitCount_[i]);
    }
  }
  void printPacket() {
    printf(" type=%d from %d, seqNum=%d\n", type, userId,
           seqNum);
    printf("H_matrix is");
    for (int i=0; i<8; i++)
      printf("%d", hitCount[i]);
  }

}__attribute__ ((packed));



class NameRequestPkt : public PacketBase {
 public:
  uint8_t targetUserId;
  char name[MAX_NAME];
  NameRequestPkt() {
  }
  NameRequestPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
                 uint8_t targetUserId_, char *name_)
      : PacketBase(NAME_REQUEST, userId_, checkSum_, seqNum_),
        targetUserId(targetUserId_) {
    int i;
    for (i = 0; i < strlen(name_) && i < MAX_NAME - 1; i++) {
      name[i] = name_[i];
    }
    name[i] = '\0';
  }

}__attribute__ ((packed));

class NameReplyPkt : public PacketBase {
 public:
  char name[MAX_NAME];
  NameReplyPkt() {
  }
  ~NameReplyPkt() {
  }
  NameReplyPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
               char *name_)
      : PacketBase(NAME_REPLY, userId_, checkSum_, seqNum_) {
    int i;
    for (i = 0; i < strlen(name_) && i < MAX_NAME - 1; i++) {
      name[i] = name_[i];
    }
    name[i] = '\0';
  }
}__attribute__ ((packed));

class GameExitPkt : public PacketBase {
 public:
  GameExitPkt() {
  }
  ~GameExitPkt() {
  }
  GameExitPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_)
      : PacketBase(GAME_EXIT, userId_, checkSum_, seqNum_) {

  }
}__attribute__ ((packed));

#endif
