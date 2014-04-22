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

  printf("argc=%d\n", argc);
  if (argc > 1) {
    ratName = (char*) malloc((unsigned) (strlen(argv[1]) + 1));
    if (ratName == NULL)
      MWError("no mem for ratName");
    strcpy(ratName, argv[1]);
    printf("name is : %s", ratName);
  } else {
    getName("Welcome to CS244B MazeWar!\n\nYour Name", &ratName);
    ratName[strlen(ratName) - 1] = 0;
  }

  M = MazewarInstance::mazewarInstanceNew(string(ratName));
  MazewarInstance* a = M.ptr();
  strncpy(M->myName_, ratName, NAMESIZE);
  free(ratName);
  MazeInit(argc, argv);

  NewPosition(M);

  //set position if there's valid input:
  if (argc > 2)
    M->xlocIs(Loc(atoi(argv[2])));
  if (argc > 3)
    M->ylocIs(Loc(atoi(argv[3])));
  if (M->maze_[MY_X_LOC][MY_Y_LOC]) {
    printf("invalid position input, reset new position\n");
    NewPosition(M);
  }

  //set direction if there's valid input:
  if (argc > 4) {
    if (strcmp(argv[4], "n") == 0) {
      M->dirIs(Direction(NORTH));
      printf("setting north\n");
    } else if (strcmp(argv[4], "s") == 0) {
      M->dirIs(Direction(SOUTH));
      printf("setting south\n");
    } else if (strcmp(argv[4], "e") == 0) {
      M->dirIs(Direction(EAST));
      printf("setting east\n");
    } else if (strcmp(argv[4], "w") == 0) {
      M->dirIs(Direction(WEST));
      printf("setting west\n");
    } else {
      printf("invalid direction\n");
    }
  }
  /* So you can see what a Rat is supposed to look like, we create
   one rat in the single player mode Mazewar.
   It doesn't move, you can't shoot it, you can just walk around it */

  play();

  return 0;
}
void printMatrix() {
  printf("the H_matrix for player %d is:\n", MY_ID);
  for (int i = 0; i < 8; i++) {
    printf("  ");
    for (int j = 0; j < 8; j++) {
      printf("%3d", M->H_matrix[i][j]);
    }
    printf("\n");
  }
}
/* ----------------------------------------------------------------------- */
bool choose_id() {
  int i, j;
//for testing corner case
//  static int x=0;
  bool success = false;
  short sum[MAX_RATS];
  for (i = 0; i < MAX_RATS; i++) {
    sum[i] = 0;
    for (j = 0; j < MAX_RATS; j++) {
      sum[i] += M->H_matrix[i][j];
    }
    if (success == false && sum[i] == -MAX_RATS) {
      M->myRatIdIs(RatId(i));
      //for testing corner case of conflict id
//      M->myRatIdIs(RatId(x++));
      success = true;
      M->H_matrix[MY_ID][MY_ID] = 0;
    }
  }
  if (success == true) {
    for (i = 0; i != MY_ID && i < MAX_RATS; i++) {
      if (sum[i] != -MAX_RATS) {
        M->H_matrix[MY_ID][i] = 0;
      }
    }
    printf("choose id done! id = %d ", MY_ID);
    printMatrix();
    return true;
  } else if (success == false) {
    printf("choose id fail! id = %d ", MY_ID);
    printMatrix();
    return false;
  }
}

void play(void) {
  MWEvent event;
  MW244BPacket incoming;
  bool needRepaint = true;

  event.eventDetail = &incoming;
  struct timeval joinTime;
  gettimeofday(&joinTime, NULL);

  while (TRUE) {
    NextEvent(&event, M->theSocket());
    switch (M->joinState()) {
      /*=================WAITING======================*/
      case WAITING:
        printf("in waiting state, waiting for 5 seconds\n");
        if (event.eventType == EVENT_NETWORK) {
          processPacket(&event);
        } else if (event.eventType == EVENT_INT) {
          quit(0);
        }
        if (isTimeOut(joinTime, JOIN_TIMEOUT)) {
          M->joinStateIs(INITING);
        }
        break;
        /*=================INITING======================*/
      case INITING:
        printf("in initing state\n");
        printMatrix();
        if (choose_id() == false) {
          M->joinStateIs(WAITING);
          break;
        }
        M->setMeInArray(MY_ID);  //set me as playing
        M->joinStateIs(PLAYING);
        sendHeartBeat();
        printf("init done, my id is %d\n", MY_ID);
        break;

        /*=================PLAYING======================*/
      case PLAYING:
        if (needRepaint) {
          needRepaint = false;
          repaintWindow();
        }
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

    for (int i = 0; i < MAX_RATS; i++) {
      if (M->mazeRats_[i].playing == true && i != MY_ID
          && isTimeOut(M->mazeRats_[i].lastHeartBeatTime, EXIT_TIMEOUT)) {
        printf("rat %d is dead \n", i);
        processGameExit(i);
      }

    }
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

  bool conflict = false;
  for (int i = 0; i < MAX_RATS; i++) {
    if (M->mazeRats_[i].playing && M->mazeRats_[i].x.value() == tx
        && M->mazeRats_[i].y.value() == ty)
      conflict = true;
  }
  if (conflict == false && ((MY_X_LOC != tx) || (MY_Y_LOC != ty))) {
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
  bool conflict = false;
  for (int i = 0; i < MAX_RATS; i++) {
    if (M->mazeRats_[i].playing && M->mazeRats_[i].x.value() == tx
        && M->mazeRats_[i].y.value() == ty)
      conflict = true;
  }
  if (conflict == false && ((MY_X_LOC != tx) || (MY_Y_LOC != ty))) {
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
  printf("quiting ===============");
  sendGameExit();
  StopWindow();
  exit(0);
}

/* ----------------------------------------------------------------------- */

void NewPosition(MazewarInstance::Ptr m) {
  Loc newX(0);
  Loc newY(0);
  Direction dir(0); /* start on occupied square */
  bool conflict = true;
  while (M->maze_[newX.value()][newY.value()] || conflict) {
    /* MAZE[XY]MAX is a power of 2 */
    newX = Loc(random() & (MAZEXMAX - 1));
    newY = Loc(random() & (MAZEYMAX - 1));
    // check that square is unoccupied by another rat
    conflict = false;
    for (int i = 0; i < MAX_RATS; i++) {
      if (m->mazeRats_[i].x.value() == newX.value()
          && m->mazeRats_[i].y.value() == newY.value())
        conflict = true;
    }
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
  uint8_t id = ratId.value();
  return M->calculateScore(id);
}

/* ----------------------------------------------------------------------- */

/* This is just for the sample version, rewrite your own */
const char *GetRatName(RatIndexType ratId) {
  return M->mazeRats_[ratId.value()].name.c_str();
}

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
    switch (MY_DIR_MIS) {
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
      printf("draw new missile\n");
      M->xMissileIs(Loc(newX));
      M->yMissileIs(Loc(newY));
      showMissile(Loc(newX), Loc(newY), MY_DIR_MIS, Loc(oldX), Loc(oldY), true);
    } else {
      //already hit the wall:
//      printf("hit the wall\n");
      M->hasMissileIs(FALSE);
      M->xMissileIs(Loc(0));
      M->yMissileIs(Loc(0));
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
  if (DEBUG) {
    float rate = (float) rand() / (float) RAND_MAX;
    if (rate < PACKET_DROP_RATE) {
      printf("dropping packet!! type = ", packet->type);
      return;
    }
  }
  if (sendto((int) M->theSocket(), (void *) packet, sizeof(MW244BPacket), 0,
             (struct sockaddr *) &groupAddr, sizeof(Sockaddr)) < 0) {
    MWError("send error");
  }
//increase sequence number here:
  M->seqNumIs(M->seqNum() + 1);
  M->mazeRats_[MY_ID].seqNum = M->seqNum();
}

void sendHeartBeat() {
  HeartBeatPkt *heartBeatPkt = new HeartBeatPkt((uint8_t) MY_ID, (uint16_t) 0,
                                                (uint32_t) M->seqNum(),
                                                MY_X_LOC,
                                                MY_Y_LOC,
                                                //12,1,
                                                MY_DIR,
                                                (int16_t) M->H_base[MY_ID],
                                                MY_X_MIS,
                                                MY_Y_MIS,
                                                (int16_t *) M->H_matrix[MY_ID]);

  heartBeatPkt->printPacket(true);
  MW244BPacket *pack = new MW244BPacket();
  memcpy(pack, heartBeatPkt, sizeof(HeartBeatPkt));
  sendPacket(pack);
  delete heartBeatPkt;
  delete pack;
  timeval cur;
  gettimeofday(&cur, NULL);
  M->lastHeartBeatTimeIs(cur);

}
void sendNameRequest(uint8_t targetUserId) {
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
  if (MY_ID >= MAX_RATS) {
    printf("exit on Joing state!!!!!");
    return;
  }
  if (M->joinState() != PLAYING)
    return;
  GameExitPkt *gameExitPkt = new GameExitPkt((uint8_t) MY_ID, (uint16_t) 0,
                                             (uint32_t) M->seqNum(),
                                             MY_X_LOC,
                                             MY_Y_LOC,
                                             MY_DIR,
                                             (int16_t) M->H_base[MY_ID],
                                             MY_X_MIS,
                                             MY_Y_MIS,
                                             (int16_t *) M->H_matrix[MY_ID]);
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
//TODO:  handle the corner case of ID conflict.
  if (packBase->userId == MY_ID && ntohl(packBase->seqNum) > M->seqNum()) {
    printf("ID conflict! reset\n");
    M->reset();
    for (int j = 0; j < MAX_RATS; j++) {
      M->mazeRats_[j].reset();
    }
    NewPosition(M);
    play();
  }
//handle the corner case of receiving packet from my self
  else if (packBase->userId == MY_ID) {
    printf("\nreceived the packet from myself, discard\n");
    return;
  }
//handle the corner case of out order packet
  else if (ntohl(packBase->seqNum) < M->mazeRats_[packBase->userId].seqNum) {
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
    case GAME_EXIT: {
      processHeartBeat((HeartBeatPkt *) pack);
      GameExitPkt * packet = (GameExitPkt *) pack;
      packet->printPacket(0);
      assert(packet->checkSumCorrect());
      processGameExit(packet->userId);
      break;
    }
    default:
      break;
  }
}
void checkCollision(HeartBeatPkt *packet) {

  short dx, dy;
  int cnt = 0;
  printf("packet->ratX =%d, MY_X_LOC =%d,  packet->ratY =%d,  MY_Y_LOC=%d\n",
  ntohs(packet->ratX),
         MY_X_LOC, ntohs(packet->ratY), MY_Y_LOC);
  if (ntohs(packet->ratX) == MY_X_LOC && ntohs(packet->ratY) == MY_Y_LOC) {
    printf("location collision!\n");
    Loc newX(0);
    Loc newY(0);
    Direction dir(0); /* start on occupied square */
    bool conflict = true;
    while (M->maze_[newX.value()][newY.value()] || conflict) {
      srand(time(NULL));
      dx = rand() % 3 - 1;
      dy = rand() % 3 - 1;
      printf("the rand dx, dy is %d, %d\n", dx, dy);
      /* MAZE[XY]MAX is a power of 2 */
      newX = Loc((MY_X_LOC + dx) & (MAZEXMAX - 1));
      newY = Loc((MY_Y_LOC + dy) & (MAZEYMAX - 1));
      // check that square is unoccupied by another rat
      conflict = false;
      for (int i = 0; i < MAX_RATS; i++) {
        if (M->mazeRats_[i].x.value() == newX.value()
            && M->mazeRats_[i].y.value() == newY.value())
          conflict = true;
      }
      if (cnt++ > 0xffffff) {
        NewPosition(M);
        break;
      }
    }
    /* prevent a blank wall at first glimpse */

    if (!M->maze_[(newX.value()) + 1][(newY.value())])
      dir = Direction(NORTH);
    if (!M->maze_[(newX.value()) - 1][(newY.value())])
      dir = Direction(SOUTH);
    if (!M->maze_[(newX.value())][(newY.value()) + 1])
      dir = Direction(EAST);
    if (!M->maze_[(newX.value())][(newY.value()) - 1])
      dir = Direction(WEST);

    M->xlocIs(newX);
    M->ylocIs(newY);
    M->dirIs(dir);
  }
}
void processHeartBeat(HeartBeatPkt *packet) {
  assert(packet->checkSumCorrect());
  checkCollision(packet);
  uint8_t id = packet->userId;
  int i;
  packet->printPacket(0);
  printMatrix();
  printf("my state = %d \n", M->joinState());

//TODO:
//if (M->H_matrix[id][id] == -1)
//I'm playing, while received heart beat from a new player:
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
    M->H_matrix[MY_ID][id] = 0;
    sendNameRequest((uint8_t) id);
  }

//update a row of H_matrix and H_base
  for (i = 0; i < MAX_RATS; i++) {
    M->H_matrix[id][i] = ntohs(packet->hitCount[i]);
  }
  M->H_base[id] = ntohs(packet->scoreBase);
//  printf("updating scorebase for %d to %d\n", id, M->H_base[id]);
//update positions
  M->mazeRats_[id].x = Loc(ntohs(packet->ratX));
  M->mazeRats_[id].y = Loc(ntohs(packet->ratY));
  M->mazeRats_[id].dir = Direction(ntohs(packet->ratD));
  M->mazeRats_[id].xMis = Loc(ntohs(packet->misX));
  M->mazeRats_[id].yMis = Loc(ntohs(packet->misY));
  M->mazeRats_[id].seqNum = ntohl(packet->seqNum);
  M->mazeRats_[id].updateHeartbeat();

//update name for new player: when I'm waiting, I set the received rat to be playing in my rat array
  if (M->joinState() == WAITING) {
    M->mazeRats_[id].playing = true;
    return;
  }
//I'm hit by rat[id]
  if (MY_X_LOC == ntohs(packet->misX) && MY_Y_LOC == ntohs(packet->misY)) {
    sendHeartBeat();
    printf("!!! attention! hit by %d, ", id);
    M->H_matrix[id][MY_ID]++;
    printMatrix();
    NewPosition(M);
  }
//I hit rat[id]
  if (MY_X_MIS == ntohs(packet->ratX) && MY_Y_MIS == ntohs(packet->ratY)) {
    printf("attention! hit %d", id);
    M->H_matrix[MY_ID][id]++;
    M->hasMissileIs(FALSE);
    clearSquare(M->xMissile(), M->yMissile());
    M->xMissileIs(Loc(0));
    M->yMissileIs(Loc(0));
    printMatrix();
    sendHeartBeat();
  }
//update score
  NewScoreCard();
  updateView = true;

}
void processNameRequest(NameRequestPkt *packet) {
  assert(packet->checkSumCorrect());
  if (packet->targetUserId != MY_ID)
    return;
  uint8_t id = packet->userId;
  packet->printPacket(0);
  M->mazeRats_[id].name = packet->name;
  sendNameReply();

}
void processNameReply(NameReplyPkt *packet) {
  assert(packet->checkSumCorrect());
  uint8_t id = packet->userId;
  packet->printPacket(0);
  M->mazeRats_[id].name = packet->name;
  M->mazeRats_[id].playing = true;
  printf("received name reply with name:%s\n", M->mazeRats_[id].name.c_str());

}
void processGameExit(uint8_t id) {
  if (id >= MAX_RATS)
    return;
//  if (M->mazeRats_[id].playing == false)
//    return;
  int i;
  printf("rat %d is exit\n", id);
  for (i = 0; i < MAX_RATS; i++) {
    //i = MY_ID;
    if (M->mazeRats_[i].playing) {
      M->H_base[i] -= (
          M->H_matrix[id][i] > 0 ? M->H_matrix[id][i] * LOSE_SCORE : 0);
      M->H_base[i] += (
          M->H_matrix[i][id] > 0 ? M->H_matrix[i][id] * WIN_SCORE : 0);
      printf("on exit detection updating scorebase for %d to %d\n", i,
             M->H_base[i]);
    }
  }
  for (i = 0; i < MAX_RATS; i++) {
    M->H_matrix[id][i] = -1;
    M->H_matrix[i][id] = -1;
  }
  M->H_base[id] = 0;
  M->mazeRats_[id].reset();
  NewScoreCard();
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
