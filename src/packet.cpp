/*
 * packet.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: songhan
 */
#include "packet.h"
//#include "mazewar.h"

//extern MazewarInstance::Ptr M;

uint16_t PacketBase::cksum(const void *_data, int len) {
  const uint8_t *data =(const uint8_t*) _data;
  uint32_t sum;

  for (sum = 0; len >= 2; data += 2, len -= 2)
    sum += data[0] << 8 | data[1];
  if (len > 0)
    sum += data[0] << 8;
  while (sum > 0xffff)
    sum = (sum >> 16) + (sum & 0xffff);
  sum = htons(~sum);
  return sum ? sum : 0xffff;
}

void PacketBase::printPacket(bool isSend) {
  int i;
  char *packetTypeStr[] = { "heart beat", "name request", "name reply",
      "game exit" };
  if (isSend) {
    printf("\n<<<<<===== Player %d sending a %s packet, seqNum = %d\n", userId,
           packetTypeStr[type], ntohl(seqNum));
  } else {
    printf("\n====>>>>>>>  received a %s packet from %d, seqNum=%d\n",
           packetTypeStr[type], userId, ntohl(seqNum));
  }

  char *aslong = (char *) this;
  for (int i = 0; i < 35; i++) {
    printf("%02x ", aslong[i]);
    if (i % 4 == 3)
      printf("\n");
  }
  printf("\n");
}

void HeartBeatPkt::printPacket(bool isSend) {

  int i;
  char *packetTypeStr[] = { "heart beat", "name request", "name reply",
      "game exit" };
  if (isSend) {
    printf("\n<<<<<===== Player %d sending a %s packet, seqNum = %d\n", userId,
           packetTypeStr[type], ntohl(seqNum));
  } else {
    printf("\n====>>>>>>>  received a %s packet from %d, seqNum=%d\n",
           packetTypeStr[type], userId, ntohl(seqNum));
  }

//  char *aslong = (char *) this;
//  for (int i = 0; i < 35; i++) {
//    printf("%02x ", aslong[i]);
//    if (i % 4 == 3)
//      printf("\n");
//  }
//  printf("\n");
  printf(
      " ratX = %hd, ratY = %hd, ratD = %hd, scoreBase = %hd, misX = %hd, misY = %hd \n hitCount=[%hd %hd %hd %hd %hd %hd %hd %hd]\n",
      ntohs(ratX),
      ntohs(ratY), ntohs(ratD),
      ntohs(scoreBase),
      ntohs(misX), ntohs(misY), ntohs(hitCount[0]), ntohs(hitCount[1]),
      ntohs(hitCount[2]),
      ntohs(hitCount[3]), ntohs(hitCount[4]),
      ntohs(hitCount[5]),
      ntohs(hitCount[6]), ntohs(hitCount[7]));
}

/*
 void HeartBeatPkt::processPacket(){
 uint8_t id = this->userId;
 int i;
 bool hit = false;
 printPacket(0);
 printf("my state = %d \n", M->joinState());

 if (M->H_matrix[id][id] == -1)
 ;
 //received heart beat from a new player:
 if ((M->mazeRats_[id].playing == false || M->mazeRats_[id].name == "")
 && M->joinState() == PLAYING) {
 short sum = 0;
 for (i = 0; i < 8; i++)
 sum += M->H_matrix[id][i];
 //    assert(sum == -8);
 sum = 0;
 for (i = 0; i < 8; i++)
 sum += M->H_matrix[i][id];
 //    assert(sum == -8);
 sendNameRequest((uint8_t) id);
 }

 //update a row of H_matrix and H_base
 for (i = 0; i < MAX_RATS; i++) {
 M->H_matrix[id][i] = ntohs(this->hitCount[i]);
 }
 M->H_base[id] = ntohs(this->scoreBase);
 //update positions
 M->mazeRats_[id].x = Loc(ntohs(this->ratX));
 M->mazeRats_[id].y = Loc(ntohs(this->ratY));
 M->mazeRats_[id].dir = Direction(ntohs(this->ratD));
 M->mazeRats_[id].xMis = Loc(ntohs(this->misX));
 M->mazeRats_[id].yMis = Loc(ntohs(this->misY));

 //update name for new player: when I'm waiting, I set the received rat to be playing in my rat array
 if (M->joinState() == WAITING) {
 M->mazeRats_[id].playing = true;
 return;
 }
 //I'm hit by rat[id]
 if (MY_X_LOC == ntohs(this->misX) && MY_Y_LOC == ntohs(this->misY)) {
 printf("hit by %d", id);
 M->H_matrix[id][MY_ID]++;
 hit = true;
 }
 //I hit rat[id]
 if (MY_X_MIS == ntohs(this->ratX) && MY_Y_MIS == ntohs(this->ratY)) {
 printf("hit %d", id);
 M->H_matrix[MY_ID][id]++;
 M->hasMissileIs(FALSE);
 M->xMissileIs(Loc(-1));
 M->yMissileIs(Loc(-1));
 hit = true;
 }
 if (hit)
 sendHeartBeat();

 //update score
 for (i = 0; i < MAX_RATS; i++) {
 M->calculateScore(i);
 }

 }
 void NameRequestPkt::processPacket(){
 if (this->targetUserId != MY_ID)
 return;
 uint8_t id = this->userId;
 this->printPacket(0);
 M->mazeRats_[id].name = this->name;
 sendNameReply();

 }
 void NameReplyPkt::processPacket(){
 uint8_t id = this->userId;
 this->printPacket(0);
 M->mazeRats_[id].name = this->name;
 M->mazeRats_[id].playing = true;
 printf("received name reply with name:%s\n", M->mazeRats_[id].name.c_str());

 }
 void GameExitPkt::processPacket(){
 uint8_t id = this->userId;
 this->printPacket(0);
 }



 */

//void HeartBeatPkt::printPacket() {
//  printf("received a heart beat (type=%d) from %d, seqNum=%d\n", type, userId,
//         seqNum);
//  printf("my state = %d \n", M->joinState());
//}
//
//void HeartBeatPkt::process() {
//  printPacket();
//}
