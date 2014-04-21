/* $Header: mazewar.h,v 1.7 88/08/25 09:59:51 kent Exp $ */

/*
 * mazewar.h - Definitions for MazeWar
 *
 * Author:	Christopher A. Kent
 * 		Western Research Laboratory
 * 		Digital Equipment Corporation
 * Date:	Wed Sep 24 1986
 */

/* Modified by Michael Greenwald for CS244B, Mar 1992,
 Greenwald@cs.stanford.edu */

/* Modified by Nicholas Dovidio for CS244B, Mar 2009,
 * ndovidio@stanford.edu
 * This version now uses the CS249a/b style of C++ coding.
 */

/***********************************************************
 Copyright 1986 by Digital Equipment Corporation, Maynard, Massachusetts,

 All Rights Reserved

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that copyright notice and this permission notice appear in
 supporting documentation, and that the names of Digital not be
 used in advertising or publicity pertaining to disstribution of the
 software without specific, written prior permission.

 DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 SOFTWARE.

 ******************************************************************/

#ifndef MAZEWAR_H
#define MAZEWAR_H

#include "fwk/NamedInterface.h"

#include "Nominal.h"
#include "Exception.h"
#include <string>
#include <time.h>
#include "packet.h"

/* fundamental constants */

#ifndef	TRUE
#define	TRUE		1
#define	FALSE		0
#endif	/* TRUE */

/* You can modify this if you want to */
#define	MAX_RATS	 8
#define MISSILE_SPEED 500
#define JOIN_TIMEOUT 2000
#define EXIT_TIMEOUT 5000
#define HEART_BEAT_RATE 500

/* network stuff */
/* Feel free to modify.  This is the simplest version we came up with */

/* A unique MAZEPORT will be assigned to your team by the TA */
#define	MAZEPORT 5005
/* The multicast group for Mazewar is 224.1.1.1 */
#define MAZEGROUP       0xe0010101
#define	MAZESERVICE	"mazewar244B"

/* The next two >must< be a power of two, because we subtract 1 from them
 to get a bitmask for random()
 */
#define	MAZEXMAX	32
#define	MAZEYMAX	16
#define	VECTORSIZE	55
#define	NAMESIZE	20
#define	NDIRECTION	4
#define	NORTH		0
#define	SOUTH		1
#define	EAST		2
#define	WEST		3
#define	NVIEW		4
#define	LEFT		0
#define	RIGHT		1
#define	REAR		2
#define	FRONT		3

#define WIN_SCORE 11
#define LOSE_SCORE 5

/* types */

typedef struct sockaddr_in Sockaddr;
typedef bool MazeRow[MAZEYMAX];
typedef MazeRow MazeType[MAZEXMAX];
typedef MazeRow *MazeTypePtr;
//typedef	short						Direction;
typedef struct {
  short x, y;
} XYpoint;
typedef struct {
  XYpoint p1, p2;
} XYpair;
typedef struct {
  short xcor, ycor;
} XY;
typedef struct {
  unsigned short bits[16];
} BitCell;
typedef char RatName[NAMESIZE];

typedef enum JoinState {
  WAITING,
  INITING,
  PLAYING
};

class Direction : public Ordinal<Direction, short> {
 public:
  Direction(short num)
      : Ordinal<Direction, short>(num) {
    if (num < NORTH || num > NDIRECTION) {
      throw RangeException("Error: Unexpected value.\n");
    }
  }
};

class Loc : public Ordinal<Loc, short> {
 public:
  Loc(short num)
      : Ordinal<Loc, short>(num) {
    if (num < 0) {
      throw RangeException("Error: Unexpected negative value.\n");
    }
  }
};

class Score : public Ordinal<Score, int> {
 public:
  Score(int num)
      : Ordinal<Score, int>(num) {
  }
};

class RatIndexType : public Ordinal<RatIndexType, int> {
 public:
  RatIndexType(int num)
      : Ordinal<RatIndexType, int>(num) {
    if (num < 0) {
      throw RangeException("Error: Unexpected negative value.\n");
    }
  }
};

class RatId : public Ordinal<RatId, unsigned char> {
 public:
  RatId(unsigned char num)
      : Ordinal<RatId, unsigned char>(num) {
  }
};

class TokenId : public Ordinal<TokenId, long> {
 public:
  TokenId(long num)
      : Ordinal<TokenId, long>(num) {
  }
};

class RatAppearance {
 public:
  RatAppearance()
      : x(1),
        y(1),
        tokenId(0) {
  }
  ;
  bool visible;
  Loc x, y;
  short distance;
  TokenId tokenId;
};

class Rat {
 public:
  Rat()
      : playing(0),
        x(1),
        y(1),
        dir(NORTH),
        xMis(0),
        yMis(0),
        hasMissile(0),
        score(0),
        id(0),
        seqNum(0) {
  }
  ;
  void setRat(bool playing_, Loc x_, Loc y_, Direction dir_, Loc xMis_,
              Loc yMis_, RatId id_, timeval lastHeartBeatTime_, int score_,
              bool hasMissile_, char * name_, uint32_t seqNum_) {
    playing = playing_;
    x = x_;
    y = y_;
    dir = dir_;
    xMis = xMis_;
    yMis = yMis_;
    id = id_;
    lastHeartBeatTime = lastHeartBeatTime_;
    score = score_;
    hasMissile = hasMissile_;
    name = name_;  //TODO
    seqNum = seqNum_;

  }
  bool playing;
  Loc x, y;
  Direction dir;
  Loc xMis, yMis;
  RatId id;
  timeval lastHeartBeatTime;
  int score;
  bool hasMissile;
  string name;
  uint32_t seqNum;

  void updateHeartbeat() {
    gettimeofday(&(this->lastHeartBeatTime), NULL);
  }
};

typedef RatAppearance RatApp_type[MAX_RATS];
typedef RatAppearance * RatLook;

/* defined in display.c */
extern RatApp_type Rats2Display;

/* variables "exported" by the mazewar "module" */
class MazewarInstance : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<MazewarInstance const> PtrConst;
  typedef Fwk::Ptr<MazewarInstance> Ptr;

  static MazewarInstance::Ptr mazewarInstanceNew(string s) {
    MazewarInstance * m = new MazewarInstance(s);
    return m;
  }

  inline Direction dir() const {
    return dir_;
  }
  void dirIs(Direction dir) {
    this->dir_ = dir;
  }
  inline Direction dirPeek() const {
    return dirPeek_;
  }
  void dirPeekIs(Direction dirPeek) {
    this->dirPeek_ = dirPeek;
  }

  inline long mazePort() const {
    return mazePort_;
  }
  void mazePortIs(long mazePort) {
    this->mazePort_ = mazePort;
  }
  inline Sockaddr* myAddr() const {
    return myAddr_;
  }
  void myAddrIs(Sockaddr *myAddr) {
    this->myAddr_ = myAddr;
  }
  inline RatId myRatId() const {
    return myRatId_;
  }
  void myRatIdIs(RatId myRatId) {
    this->myRatId_ = myRatId;
  }

  inline bool peeking() const {
    return peeking_;
  }
  void peekingIs(bool peeking) {
    this->peeking_ = peeking;
  }
  inline int theSocket() const {
    return theSocket_;
  }
  void theSocketIs(int theSocket) {
    this->theSocket_ = theSocket;
  }
  inline Score score() const {
    return score_;
  }
  void scoreIs(Score score) {
    this->score_ = score;
  }
  inline Loc xloc() const {
    return xloc_;
  }
  void xlocIs(Loc xloc) {
    this->xloc_ = xloc;
  }
  inline Loc yloc() const {
    return yloc_;
  }
  void ylocIs(Loc yloc) {
    this->yloc_ = yloc;
  }
  inline Loc xPeek() const {
    return xPeek_;
  }
  void xPeekIs(Loc xPeek) {
    this->xPeek_ = xPeek;
  }
  inline Loc yPeek() const {
    return yPeek_;
  }
  void yPeekIs(Loc yPeek) {
    this->yPeek_ = yPeek;
  }
  inline int active() const {
    return active_;
  }
  void activeIs(int active) {
    this->active_ = active;
  }

  inline bool hasMissile() const {
    return hasMissile_;
  }
  void hasMissileIs(bool hasMissile) {
    this->hasMissile_ = hasMissile;
  }
  inline Loc xMissile() const {
    return xMissile_;
  }
  void xMissileIs(Loc xMissile) {
    this->xMissile_ = xMissile;
  }
  inline Loc yMissile() const {
    return yMissile_;
  }
  void yMissileIs(Loc yMissile) {
    this->yMissile_ = yMissile;
  }
  inline Direction dirMissile() const {
    return dirMissile_;
  }
  void dirMissileIs(Direction dirMissile) {
    this->dirMissile_ = dirMissile;
  }
  inline timeval lastUpdateTime() {
    return lastMisUpdateTime_;
  }
  void lastUpdateTimeIs(timeval lastUpdateTime) {
    this->lastMisUpdateTime_ = lastUpdateTime;
  }
  inline timeval lastHeartBeatTime() {
    return lastHeartBeatTime_;
  }
  void lastHeartBeatTimeIs(timeval lastHeartBeatTime) {
    this->lastHeartBeatTime_ = lastHeartBeatTime;
  }

  inline Rat rat(RatIndexType num) const {
    return mazeRats_[num.value()];
  }
  void ratIs(Rat rat, RatIndexType num) {
    this->mazeRats_[num.value()] = rat;
  }
  inline JoinState joinState() const {
    return this->joinState_;
  }
  void joinStateIs(JoinState joinState) {
    this->joinState_ = joinState;
  }
  inline uint32_t seqNum() const {
    return this->seqNum_;
  }
  void seqNumIs(uint32_t seqNum) {
    this->seqNum_ = seqNum;
  }
  Score calculateScore(int ratId) {
    int16_t score = H_base[ratId];
//    printf("updating score card: score = %d \n", score);
//    for (int i = 0; i < 8; i++) {
//      for (int j = 0; j < 8; j++) {
//        printf("%3d ", H_matrix[i][j]);
//      }
//      printf("\n");
//    }
    for (int i = 0; i < MAX_RATS; i++) {
      if (i == ratId)
        continue;
      score += (H_matrix[ratId][i] > 0 ? WIN_SCORE * H_matrix[ratId][i] : 0);
//      printf("H[ratid][i]=%d ", H_matrix[ratId][i]);
//      printf("i = %d, score = %d \n", i, score);
      score -= (H_matrix[i][ratId] > 0 ? LOSE_SCORE * H_matrix[i][ratId] : 0);
//      printf("H[i][ratid]=%d ", H_matrix[i][ratId]);
//      printf("i = %d, score = %d ; ", i, score);
    }
    score -= H_matrix[ratId][ratId];
    printf("final score = %d\n", score);
    mazeRats_[ratId].score = score;
    return Score(score);
  }
  void setMeInArray(int ratId) {
    mazeRats_[ratId].playing = true;
    mazeRats_[ratId].x = xloc_;
    mazeRats_[ratId].y = yloc_;
    mazeRats_[ratId].dir = dir_;
    mazeRats_[ratId].xMis = xMissile_;
    mazeRats_[ratId].yMis = yMissile_;
    mazeRats_[ratId].id = myRatId_;
    mazeRats_[ratId].lastHeartBeatTime = lastMisUpdateTime_;
    mazeRats_[ratId].score = score_.value();
    mazeRats_[ratId].hasMissile = hasMissile_;
    mazeRats_[ratId].name = name();
    mazeRats_[ratId].seqNum = seqNum_;

  }

 protected:
  MazewarInstance(string s)
      : Fwk::NamedInterface(s),
        dir_(0),
        dirPeek_(0),
        myRatId_(0xFE),
        score_(0),
        xloc_(1),
        yloc_(3),
        xPeek_(0),
        yPeek_(0),
        hasMissile_(0),
        xMissile_(0),
        yMissile_(0),
        dirMissile_(0),
        joinState_(WAITING),
        seqNum_(0) {
    myAddr_ = (Sockaddr*) malloc(sizeof(Sockaddr));
    if (!myAddr_) {
      printf("Error allocating sockaddr variable");
    }
    int i, j;
    for (i = 0; i < MAX_RATS; i++) {
      printf("initializing M!!!!!=================\n");
      H_base[i] = 0;
      H_occupied[i] = false;
      for (j = 0; j < MAX_RATS; j++) {
        H_matrix[i][j] = -1;
      }
    }
  }

  long mazePort_;
  Sockaddr *myAddr_;

  RatId myRatId_;
  Direction dir_;
  Direction dirPeek_;

  bool peeking_;
  int theSocket_;
  Score score_;
  Loc xloc_;
  Loc yloc_;
  Loc xPeek_;
  Loc yPeek_;
  bool active_;

  bool hasMissile_;
  Loc xMissile_;
  Loc yMissile_;
  Direction dirMissile_;
  timeval lastMisUpdateTime_;
  timeval lastHeartBeatTime_;
  uint32_t seqNum_;
  JoinState joinState_;
 public:
  MazeType maze_;
  RatName myName_;
  int16_t H_matrix[MAX_RATS][MAX_RATS];
  int16_t H_base[MAX_RATS];
  bool H_occupied[MAX_RATS];
  Rat mazeRats_[MAX_RATS];

};
extern MazewarInstance::Ptr M;

#define MY_RAT_INDEX		M->myRatId().value()
#define MY_DIR			M->dir().value()
#define MY_DIR_MIS M->dirMissile().value()
#define MY_X_LOC		M->xloc().value()
#define MY_Y_LOC		M->yloc().value()
#define MY_X_MIS  M->xMissile().value()
#define MY_Y_MIS  M->yMissile().value()
#define MY_ID M->myRatId().value()
/* events */

#define	EVENT_A		1		/* user pressed "A" */
#define	EVENT_S		2		/* user pressed "S" */
#define	EVENT_F		3		/* user pressed "F" */
#define	EVENT_D		4		/* user pressed "D" */
#define	EVENT_BAR	5		/* user pressed space bar */
#define	EVENT_LEFT_D	6		/* user pressed left mouse button */
#define	EVENT_RIGHT_D	7		/* user pressed right button */
#define	EVENT_MIDDLE_D	8		/* user pressed middle button */
#define	EVENT_LEFT_U	9		/* user released l.M.b */
#define	EVENT_RIGHT_U	10		/* user released r.M.b */

#define	EVENT_NETWORK	16		/* incoming network packet */
#define	EVENT_INT	17		/* user pressed interrupt key */
#define	EVENT_TIMEOUT	18		/* nothing happened! */

extern unsigned short ratBits[];
/* replace this with appropriate definition of your own */
typedef struct {
  uint8_t type;
  uint8_t body[256];
} MW244BPacket;

typedef struct {
  short eventType;
  MW244BPacket *eventDetail; /* for incoming data */
  Sockaddr eventSource;
} MWEvent;

void *malloc();
Sockaddr *resolveHost();

/* display.c */
void InitDisplay(int, char **);
void StartDisplay(void);
void ShowView(Loc, Loc, Direction);
void SetMyRatIndexType(RatIndexType);
void SetRatPosition(RatIndexType, Loc, Loc, Direction);
void ClearRatPosition(RatIndexType);
void ShowPosition(Loc, Loc, Direction);
void ShowAllPositions(void);
void showMe(Loc, Loc, Direction);
void clearPosition(RatIndexType, Loc, Loc);
void clearSquare(Loc xClear, Loc yClear);
void NewScoreCard(void);
void UpdateScoreCard(RatIndexType);
void FlipBitmaps(void);
void bitFlip(BitCell *, int size);
void SwapBitmaps(void);
void byteSwap(BitCell *, int size);
void showMissile(Loc, Loc, Direction, Loc, Loc, bool);
void clearSquare(Loc, Loc);

/* init.c */
void MazeInit(int, char **);
void ratStates(void);
void getMaze(void);
void setRandom(void);
void getName(char *, char **);
void getString(char *, char **);
void getHostName(char *, char **, Sockaddr *);
Sockaddr *resolveHost(char *);
bool emptyAhead();
bool emptyRight();
bool emptyLeft();
bool emptyBehind();

/* toplevel.c */
void play(void);
void aboutFace(void);
void leftTurn(void);
void rightTurn(void);
void forward(void);
void backward(void);
void peekLeft(void);
void peekRight(void);
void peekStop(void);
void shoot(void);
void quit(int);
void NewPosition(MazewarInstance::Ptr M);
void MWError(char *);
Score GetRatScore(RatIndexType);
const char *GetRatName(RatIndexType);
void ConvertIncoming(MW244BPacket *);
void ConvertOutgoing(MW244BPacket *);
void ratState(void);
void manageMissiles(void);
void DoViewUpdate(void);
void sendPacketToPlayer(RatId);
void processPacket(MWEvent *);
void processHeartBeat(HeartBeatPkt *);
void processNameRequest(NameRequestPkt *);
void processNameReply(NameReplyPkt *);
void processGameExit(GameExitPkt *);

void netInit(void);
bool isTimeOut(timeval, long);
void sendPacket(PacketBase *);
void sendHeartBeat();
void sendNameRequest(uint8_t);
void sendNameReply();
void sendGameExit();

/* winsys.c */
void InitWindow(int, char **);
void StartWindow(int, int);
void ClearView(void);
void DrawViewLine(int, int, int, int);
void NextEvent(MWEvent *, int);
bool KBEventPending(void);
void HourGlassCursor(void);
void RatCursor(void);
void DeadRatCursor(void);
void HackMazeBitmap(Loc, Loc, BitCell *);
void DisplayRatBitmap(int, int, int, int, int, int);
void WriteScoreString(RatIndexType);
void ClearScoreLine(RatIndexType);
void InvertScoreLine(RatIndexType);
void NotifyPlayer(void);
void DrawString(const char*, uint32_t, uint32_t, uint32_t);
void StopWindow(void);
void repaintWindow();

#endif
