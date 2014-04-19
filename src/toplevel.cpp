/*
 *   FILE: toplevel.c
 * AUTHOR: name (email)
 *   DATE: March 31 23:59:59 PST 2013
 *  DESCR:
 */

/* #define DEBUG */

#include <string>
#include "main.h"
#include "mazewar.h"

static bool updateView; /* true if update needed */
MazewarInstance::Ptr M;

/* Use this socket address to send packets to the multi-cast group. */
static Sockaddr groupAddr;
#define MAX_OTHER_RATS  (MAX_RATS - 1)

int main(int argc, char *argv[]) {
  Loc x(1);
  Loc y(5);
  Direction dir(0);
  char *ratName;

  signal(SIGHUP, quit);
  signal(SIGINT, quit);
  signal(SIGTERM, quit);

  getName("Welcome to CS244B MazeWar!\n\nYour Name", &ratName);
  ratName[strlen(ratName) - 1] = 0;

  M = MazewarInstance::mazewarInstanceNew(string(ratName));
  MazewarInstance* a = M.ptr();
  strncpy(M->myName_, ratName, NAMESIZE);
  free(ratName);

  MazeInit(argc, argv);

  NewPosition(M);

  /* So you can see what a Rat is supposed to look like, we create
   one rat in the single player mode Mazewar.
   It doesn't move, you can't shoot it, you can just walk around it */

  play();

  return 0;
}

/* ----------------------------------------------------------------------- */
void choose_id() {
  int i, j;
  short sum[MAX_RATS];
  for (i = 0; i < MAX_RATS; i++) {
    sum[i] = 0;
    for (j = 0; j < MAX_RATS; j++) {
//      printf("in choose id, H[%d][%d] is: %d \n", i, j, M->H_matrix[i][j]);
      sum[i] += M->H_matrix[i][j];
    }
  }
  for (i = 0; i < MAX_RATS; i++) {
    if (sum[i] == -MAX_RATS) {
      M->myRatIdIs(RatId(i));
      M->H_matrix[i][MY_ID] = 0;
      M->H_matrix[MY_ID][i] = 0;
      break;
    }
  }
}
void play(void) {
  MWEvent event;
  MW244BPacket incoming;

  event.eventDetail = &incoming;
  struct timeval joinTime;
  gettimeofday(&joinTime, NULL);

  while (TRUE) {
    NextEvent(&event, M->theSocket());
    switch (M->joinState()) {
      /*=================WAITING======================*/
      case WAITING:
        printf("in waiting state, waiting for 3 seconds\n");
        if (event.eventType == EVENT_NETWORK) {
          processPacket(&event);
        }
        if (isTimeOut(joinTime, JOIN_TIMEOUT)) {
          M->joinStateIs(INITING);
        }
        break;
        /*=================INITING======================*/
      case INITING:
        printf("in initing state\n");
        choose_id();
        M->setMeInArray(MY_ID);  //set me as playing
        M->joinStateIs(PLAYING);
        sendHeartBeat();
        printf("init done, my id is %d\n", MY_ID);
        break;

        /*=================PLAYING======================*/
      case PLAYING:
        repaintWindow();
        if (!M->peeking())
          switch (event.eventType) {
            case EVENT_A:
              aboutFace();
              sendHeartBeat();
              break;

            case EVENT_S:
              leftTurn();
              sendHeartBeat();
              break;

            case EVENT_D:
              forward();
              sendHeartBeat();
              break;

            case EVENT_F:
              rightTurn();
              sendHeartBeat();
              break;

            case EVENT_BAR:
              backward();
              sendHeartBeat();
              break;

            case EVENT_LEFT_D:
              peekLeft();
              break;

            case EVENT_MIDDLE_D:
              shoot();
              sendHeartBeat();
              break;

            case EVENT_RIGHT_D:
              peekRight();
              break;

            case EVENT_NETWORK:
              processPacket(&event);
              break;

            case EVENT_INT:
              quit(0);
              break;
            default:
              break;
          }
        else
          switch (event.eventType) {
            case EVENT_RIGHT_U:
            case EVENT_LEFT_U:
              peekStop();
              break;

            case EVENT_NETWORK:
              processPacket(&event);
              break;
            default:
              break;
          }
        break;
      default:
        break;
    }

    ratStates(); /* clean house */

    manageMissiles();

    DoViewUpdate();

    if (M->joinState() == PLAYING
        && isTimeOut(M->lastHeartBeatTime(), HEART_BEAT_RATE))
      sendHeartBeat();
  }
}

/* ----------------------------------------------------------------------- */

static Direction _aboutFace[NDIRECTION] = { SOUTH, NORTH, WEST, EAST };
static Direction _leftTurn[NDIRECTION] = { WEST, EAST, NORTH, SOUTH };
static Direction _rightTurn[NDIRECTION] = { EAST, WEST, SOUTH, NORTH };

void aboutFace(void) {
  M->dirIs(_aboutFace[MY_DIR]);
  updateView = TRUE;
}

/* ----------------------------------------------------------------------- */

void leftTurn(void) {
  M->dirIs(_leftTurn[MY_DIR]);
  updateView = TRUE;
}

/* ----------------------------------------------------------------------- */

void rightTurn(void) {
  M->dirIs(_rightTurn[MY_DIR]);
  updateView = TRUE;
}

/* ----------------------------------------------------------------------- */

/* remember ... "North" is to the right ... positive X motion */

void forward(void) {
  register int tx = MY_X_LOC;
  register int ty = MY_Y_LOC;

  switch (MY_DIR) {
    case NORTH:
      if (!M->maze_[tx + 1][ty])
        tx++;
      break;
    case SOUTH:
      if (!M->maze_[tx - 1][ty])
        tx--;
      break;
    case EAST:
      if (!M->maze_[tx][ty + 1])
        ty++;
      break;
    case WEST:
      if (!M->maze_[tx][ty - 1])
        ty--;
      break;
    default:
      MWError("bad direction in Forward");
  }
  if ((MY_X_LOC != tx) || (MY_Y_LOC != ty)) {
    M->xlocIs(Loc(tx));
    M->ylocIs(Loc(ty));
    updateView = TRUE;
  }
}

/* ----------------------------------------------------------------------- */

void backward() {
  register int tx = MY_X_LOC;
  register int ty = MY_Y_LOC;

  switch (MY_DIR) {
    case NORTH:
      if (!M->maze_[tx - 1][ty])
        tx--;
      break;
    case SOUTH:
      if (!M->maze_[tx + 1][ty])
        tx++;
      break;
    case EAST:
      if (!M->maze_[tx][ty - 1])
        ty--;
      break;
    case WEST:
      if (!M->maze_[tx][ty + 1])
        ty++;
      break;
    default:
      MWError("bad direction in Backward");
  }
  if ((MY_X_LOC != tx) || (MY_Y_LOC != ty)) {
    M->xlocIs(Loc(tx));
    M->ylocIs(Loc(ty));
    updateView = TRUE;
  }
}

/* ----------------------------------------------------------------------- */

void peekLeft() {
  M->xPeekIs(MY_X_LOC);
  M->yPeekIs(MY_Y_LOC);
  M->dirPeekIs(MY_DIR);

  switch (MY_DIR) {
    case NORTH:
      if (!M->maze_[MY_X_LOC + 1][MY_Y_LOC]) {
        M->xPeekIs(MY_X_LOC + 1);
        M->dirPeekIs(WEST);
      }
      break;

    case SOUTH:
      if (!M->maze_[MY_X_LOC - 1][MY_Y_LOC]) {
        M->xPeekIs(MY_X_LOC - 1);
        M->dirPeekIs(EAST);
      }
      break;

    case EAST:
      if (!M->maze_[MY_X_LOC][MY_Y_LOC + 1]) {
        M->yPeekIs(MY_Y_LOC + 1);
        M->dirPeekIs(NORTH);
      }
      break;

    case WEST:
      if (!M->maze_[MY_X_LOC][MY_Y_LOC - 1]) {
        M->yPeekIs(MY_Y_LOC - 1);
        M->dirPeekIs(SOUTH);
      }
      break;

    default:
      MWError("bad direction in PeekLeft");
  }

  /* if any change, display the new view without moving! */

  if ((M->xPeek() != MY_X_LOC) || (M->yPeek() != MY_Y_LOC)) {
    M->peekingIs(TRUE);
    updateView = TRUE;
  }
}

/* ----------------------------------------------------------------------- */

void peekRight() {
  M->xPeekIs(MY_X_LOC);
  M->yPeekIs(MY_Y_LOC);
  M->dirPeekIs(MY_DIR);

  switch (MY_DIR) {
    case NORTH:
      if (!M->maze_[MY_X_LOC + 1][MY_Y_LOC]) {
        M->xPeekIs(MY_X_LOC + 1);
        M->dirPeekIs(EAST);
      }
      break;

    case SOUTH:
      if (!M->maze_[MY_X_LOC - 1][MY_Y_LOC]) {
        M->xPeekIs(MY_X_LOC - 1);
        M->dirPeekIs(WEST);
      }
      break;

    case EAST:
      if (!M->maze_[MY_X_LOC][MY_Y_LOC + 1]) {
        M->yPeekIs(MY_Y_LOC + 1);
        M->dirPeekIs(SOUTH);
      }
      break;

    case WEST:
      if (!M->maze_[MY_X_LOC][MY_Y_LOC - 1]) {
        M->yPeekIs(MY_Y_LOC - 1);
        M->dirPeekIs(NORTH);
      }
      break;

    default:
      MWError("bad direction in PeekRight");
  }

  /* if any change, display the new view without moving! */

  if ((M->xPeek() != MY_X_LOC) || (M->yPeek() != MY_Y_LOC)) {
    M->peekingIs(TRUE);
    updateView = TRUE;
  }
}

/* ----------------------------------------------------------------------- */

void peekStop() {
  M->peekingIs(FALSE);
  updateView = TRUE;
}

/* ----------------------------------------------------------------------- */

void shoot() {
//TODO

  if (M->hasMissile()) {
    return;
  } else {
    M->hasMissileIs(TRUE);
    M->dirMissileIs(M->dir());
    M->xMissileIs(M->xloc());
    M->yMissileIs(M->yloc());

    timeval t;
    gettimeofday(&t, NULL);
    M->lastUpdateTimeIs(t);

    M->H_matrix[MY_ID][MY_ID] += 1;

    M->scoreIs(M->calculateScore(MY_ID));

    UpdateScoreCard(MY_ID);

  }
  updateView = TRUE;

}

/* ----------------------------------------------------------------------- */

/*
 * Exit from game, clean up window
 */

void quit(int sig) {
  StopWindow();
  exit(0);
}

/* ----------------------------------------------------------------------- */

void NewPosition(MazewarInstance::Ptr m) {
  Loc newX(0);
  Loc newY(0);
  Direction dir(0); /* start on occupied square */

  while (M->maze_[newX.value()][newY.value()]) {
    /* MAZE[XY]MAX is a power of 2 */
    newX = Loc(random() & (MAZEXMAX - 1));
    newY = Loc(random() & (MAZEYMAX - 1));

//TODO
    /* In real game, also check that square is
     unoccupied by another rat */
  }

  /* prevent a blank wall at first glimpse */

  if (!m->maze_[(newX.value()) + 1][(newY.value())])
    dir = Direction(NORTH);
  if (!m->maze_[(newX.value()) - 1][(newY.value())])
    dir = Direction(SOUTH);
  if (!m->maze_[(newX.value())][(newY.value()) + 1])
    dir = Direction(EAST);
  if (!m->maze_[(newX.value())][(newY.value()) - 1])
    dir = Direction(WEST);

  m->xlocIs(newX);
  m->ylocIs(newY);
  m->dirIs(dir);
}

/* ----------------------------------------------------------------------- */

void MWError(char *s) {
  StopWindow();
  fprintf(stderr, "CS244BMazeWar: %s\n", s);
  perror("CS244BMazeWar");
  exit(-1);
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own */
Score GetRatScore(RatIndexType ratId) {
  short id = ratId.value();
  return M->calculateScore(id);
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own */
const char *GetRatName(RatIndexType ratId) {
  if (ratId.value() == M->myRatId().value()) {
    return (M->myName_);
  } else {
    return M->mazeRats_[ratId.value()].name.c_str();
  }
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own if necessary */
void ConvertIncoming(MW244BPacket *p) {
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own if necessary */
void ConvertOutgoing(MW244BPacket *p) {
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own */
void ratStates() {
//TODO
  /* In our sample version, we don't know about the state of any rats over
   the net, so this is a no-op */
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own */
void manageMissiles() {
//TODO
  if (M->hasMissile() == FALSE)
    return;

  timeval pre = M->lastUpdateTime();
  timeval now;
  gettimeofday(&now, NULL);
  if (isTimeOut(pre, MISSILE_SPEED)) {  //it's time to forward the missile
    M->lastUpdateTimeIs(now);
    int oldX = MY_X_MIS;
    int oldY = MY_Y_MIS;
    int newX = oldX;
    int newY = oldY;
//    printf("oldX=%d, oldY=%d\n", oldX, oldY);
    switch (MY_DIR) {
      case NORTH:
        newX = oldX + 1;
        break;
      case SOUTH:
        newX = oldX - 1;
        break;
      case EAST:
        newY = oldY + 1;
        break;
      case WEST:
        newY = oldY - 1;
        break;
      default:
        MWError("bad direction in Forward");
    }
//    printf("direction is %d \n", MY_DIR);
//    printf("oldX=%d, oldY=%d\n", oldX, oldY);
//    printf("newX=%d, newY=%d\n", newX, newY);
    if (!M->maze_[newX][newY]) {
//      printf("draw new missile\n");
      M->xMissileIs(Loc(newX));
      M->yMissileIs(Loc(newY));
      showMissile(Loc(newX), Loc(newY), MY_DIR_MIS, Loc(oldX), Loc(oldY), true);
    } else {
      //already hit the wall:
//      printf("hit the wall\n");
      M->hasMissileIs(FALSE);
      clearSquare(Loc(oldX), Loc(oldY));
    }
    updateView = TRUE;

  }
}

/* ----------------------------------------------------------------------- */

void DoViewUpdate() {
  if (updateView) { /* paint the screen */
    ShowPosition(MY_X_LOC, MY_Y_LOC, MY_DIR);
    if (M->peeking())
      ShowView(M->xPeek(), M->yPeek(), M->dirPeek());
    else
      ShowView(MY_X_LOC, MY_Y_LOC, MY_DIR);
    updateView = FALSE;
  }
}

/* ----------------------------------------------------------------------- */

/*
 * Sample code to send a packet to a specific destination
 */

/*
 * Notice the call to ConvertOutgoing.  You might want to call ConvertOutgoing
 * before any call to sendto.
 */

void sendPacketToPlayer(RatId ratId) {
//
//  MW244BPacket pack;
//  HeartBeatPkt *packX;
//
//  pack.type = PACKET_TYPE_X;
//  packX = (HeartBeatPkt *) &pack.body;
//  packX->foo = d1;
//  packX->bar = d2;
//
//  ....
//
//  ConvertOutgoing(pack);
//
//  if (sendto((int) mySocket, &pack, sizeof(pack), 0, (Sockaddr) destSocket,
//             sizeof(Sockaddr)) < 0) {
//    MWError("Sample error")
//  };

}

void sendPacket(MW244BPacket *packet) {
// TODO:
//  if (DEBUG) {
//    float rate = (float) rand() / (float) RAND_MAX;
//    if (rate < PACKET_DROP_RATE) {
//      delete pack;
//      return;
//    }
//  }
  if (sendto((int) M->theSocket(), (void *) packet, sizeof(MW244BPacket), 0,
             (struct sockaddr *) &groupAddr, sizeof(Sockaddr)) < 0) {
    MWError("send error");
  }
  M->seqNumIs(M->seqNum() + 1);
}

void sendHeartBeat() {
//  HeartBeatPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
//                 int16_t ratX_, int16_t ratY_, int16_t ratD_, int16_t scoreBase_,
//                 int16_t misX_, int16_t misY_, int16_t *hitCount_);
  HeartBeatPkt *heartBeatPkt = new HeartBeatPkt((uint8_t) MY_ID, (uint16_t) 0,
                                                (uint32_t) M->seqNum(),
                                                MY_X_LOC,
                                                MY_Y_LOC,
                                                MY_DIR,
                                                (int16_t) M->H_base[MY_ID],
                                                MY_X_MIS,
                                                MY_Y_MIS,
                                                (int16_t *) M->H_matrix[MY_ID]);

  heartBeatPkt->printPacket(true);
  MW244BPacket *pack = new MW244BPacket();
  memcpy(pack, heartBeatPkt, sizeof(HeartBeatPkt));
  char *aslong = (char *) pack;
  sendPacket(pack);
  delete heartBeatPkt;
  delete pack;
  timeval cur;
  gettimeofday(&cur, NULL);
  M->lastHeartBeatTimeIs(cur);

}

void sendNameRequest(uint8_t targetUserId) {
//  NameRequestPkt(uint8_t userId_, uint16_t checkSum_, uint32_t seqNum_,
//                   uint8_t targetUserId_, char *name_);
  NameRequestPkt *nameRequestPkt = new NameRequestPkt((uint8_t) MY_ID,
                                                      (uint16_t) 0,
                                                      (uint32_t) M->seqNum(),
                                                      (uint8_t) targetUserId,
                                                      M->name().c_str());
  nameRequestPkt->printPacket(1);
  MW244BPacket *pack = new MW244BPacket();
  memcpy(pack, nameRequestPkt, sizeof(NameRequestPkt));
  sendPacket(pack);
  delete nameRequestPkt;
  delete pack;
}
void sendNameReply() {
  NameReplyPkt *nameReplyPkt = new NameReplyPkt((uint8_t) MY_ID, (uint16_t) 0,
                                                (uint32_t) M->seqNum(),
                                                M->name().c_str());
  nameReplyPkt->printPacket(1);
  MW244BPacket *pack = new MW244BPacket();
  memcpy(pack, nameReplyPkt, sizeof(NameReplyPkt));
  sendPacket(pack);
  delete nameReplyPkt;
  delete pack;

}

void sendGameExit() {
  GameExitPkt *gameExitPkt = new GameExitPkt();
  gameExitPkt->printPacket(1);
  MW244BPacket *pack = new MW244BPacket();
  memcpy(pack, gameExitPkt, sizeof(GameExitPkt));
  sendPacket(pack);
  delete gameExitPkt;
  delete pack;
}

/* ----------------------------------------------------------------------- */

/* Sample of processPacket. */

void processPacket(MWEvent *eventPacket) {
  MW244BPacket *pack = eventPacket->eventDetail;
  PacketBase *packBase = (PacketBase*) pack;
  if (packBase->userId == MY_ID) {
    printf("\nreceived the packet from myself, discard\n");
    return;
  }
  if (packBase->seqNum < M->mazeRats_[packBase->userId].seqNum) {
    printf("received out of order packet, discard\n");
    return;
  }
  switch (pack->type) {
    case HEART_BEAT:
      processHeartBeat((HeartBeatPkt *) pack);
      break;
    case NAME_REQUEST:
      processNameRequest((NameRequestPkt *) pack);
      break;
    case NAME_REPLY:
      processNameReply((NameReplyPkt *) pack);
      break;
    case GAME_EXIT:
      processGameExit((GameExitPkt *) pack);
      break;
    default:
      break;
  }
}

void processHeartBeat(HeartBeatPkt *packet) {
  int id = packet->userId;
  int i;
  packet->printPacket(0);
  printf("my state = %d \n", M->joinState());

  if (M->H_matrix[id][id] == -1)
    ;
  //received heart beat from a new player:
  if ((M->mazeRats_[id].playing == false || M->mazeRats_[id].name == "" ) && M->joinState() == PLAYING) {
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

//update name for new player
  if (M->joinState() == WAITING){
    M->mazeRats_[id].playing = true;
  }

//update a row of H_matrix and H_base
  for (i = 0; i < MAX_RATS; i++) {
    M->H_matrix[id][i] = ntohs(packet->hitCount[i]);
  }
  M->H_base[id] = ntohs(packet->scoreBase);
//update positions
  M->mazeRats_[id].x = Loc(ntohs(packet->ratX));
  M->mazeRats_[id].y = Loc(ntohs(packet->ratY));
  M->mazeRats_[id].dir = Direction(ntohs(packet->ratD));
  M->mazeRats_[id].xMis = Loc(ntohs(packet->misX));
  M->mazeRats_[id].yMis = Loc(ntohs(packet->misY));
//update score
  for (i = 0; i < MAX_RATS; i++) {
    M->calculateScore(i);
  }

}
void processNameRequest(NameRequestPkt *packet) {
  if (packet->targetUserId != MY_ID)
    return;
  int id = packet->userId;
  int i;
  packet->printPacket(0);
  M->mazeRats_[id].name=M->name();
  sendNameReply();

}
void processNameReply(NameReplyPkt *packet) {
  int id = packet->userId;
  int i;
  packet->printPacket(0);
  M->mazeRats_[id].name = packet->name;
  M->mazeRats_[id].playing = true;
  printf("received name reply with name:%s\n", M->mazeRats_[id].name.c_str());

}
void processGameExit(GameExitPkt *packet) {
  int id = packet->userId;
  int i;
  packet->printPacket(0);

}

/* ----------------------------------------------------------------------- */

/* This will presumably be modified by you.
 It is here to provide an example of how to open a UDP port.
 You might choose to use a different strategy
 */
//TODO
void netInit() {
  Sockaddr nullAddr;
  Sockaddr *thisHost;
  char buf[128];
  int reuse;
  u_char ttl;
  struct ip_mreq mreq;

  /* MAZEPORT will be assigned by the TA to each team */
  M->mazePortIs(htons(MAZEPORT));

  gethostname(buf, sizeof(buf));
  if ((thisHost = resolveHost(buf)) == (Sockaddr *) NULL)
    MWError("who am I?");
  bcopy((caddr_t) thisHost, (caddr_t) (M->myAddr()), sizeof(Sockaddr));

  M->theSocketIs(socket(AF_INET, SOCK_DGRAM, 0));
  if (M->theSocket() < 0)
    MWError("can't get socket");

  /* SO_REUSEADDR allows more than one binding to the same
   socket - you cannot have more than one player on one
   machine without this */
  reuse = 1;
  if (setsockopt(M->theSocket(), SOL_SOCKET, SO_REUSEADDR, &reuse,
                 sizeof(reuse)) < 0) {
    MWError("setsockopt failed (SO_REUSEADDR)");
  }

  nullAddr.sin_family = AF_INET;
  nullAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  nullAddr.sin_port = M->mazePort();
  if (bind(M->theSocket(), (struct sockaddr *) &nullAddr, sizeof(nullAddr)) < 0)
    MWError("netInit binding");

  /* Multicast TTL:
   0 restricted to the same host
   1 restricted to the same subnet
   32 restricted to the same site
   64 restricted to the same region
   128 restricted to the same continent
   255 unrestricted

   DO NOT use a value > 32. If possible, use a value of 1 when
   testing.
   */

  ttl = 1;
  if (setsockopt(M->theSocket(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
                 sizeof(ttl)) < 0) {
    MWError("setsockopt failed (IP_MULTICAST_TTL)");
  }

  /* join the multicast group */
  mreq.imr_multiaddr.s_addr = htonl(MAZEGROUP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(M->theSocket(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq,
                 sizeof(mreq)) < 0) {
    MWError("setsockopt failed (IP_ADD_MEMBERSHIP)");
  }

  /*
   * Now we can try to find a game to join; if none, start one.
   */

  printf("\n");

  /* set up some stuff strictly for this local sample */
//TODO:
//  M->myRatIdIs(0);
//  M->scoreIs(0);
//  SetMyRatIndexType(0);
  /* Get the multi-cast address ready to use in SendData()
   calls. */
  memcpy(&groupAddr, &nullAddr, sizeof(Sockaddr));
  groupAddr.sin_addr.s_addr = htonl(MAZEGROUP);
}

/* ----------------------------------------------------------------------- */
bool isTimeOut(timeval oldTime, long timeOut) {
  struct timeval curTime;
  gettimeofday(&curTime, NULL);
  if ((curTime.tv_sec - oldTime.tv_sec) * 1000
      + (curTime.tv_usec - oldTime.tv_usec) / 1000 > timeOut)
    return true;
  else
    return false;
}
