/*
 * networkInstance.h
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#ifndef NETWORKINSTANCE_H_
#define NETWORKINSTANCE_H_

#include "debug.h"
#include "packet.h"
#include "stdlib.h"
#include "stdio.h"
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> //contains AF_INET
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <poll.h>

#define PORT 44024
#define GROUP 0xe0010101

#define POLL_TIME_OUT 500

class Network {
 public:
  Network(int, int, int);
  virtual ~Network();
  int send(PacketBase *p);    //
  PacketBase * receive(); //return the pointer to a packet

  int group;
  int port;
  int dropRate;

 private:
  int mySocket;
  struct sockaddr_in myAddr;
  struct sockaddr_in groupAddr;
  struct sockaddr_in* resolveHost(register char *);
};
#endif /* NETWORKINSTANCE_H_ */

