/*
 * networkInstance.h
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#ifndef NETWORKINSTANCE_H_
#define NETWORKINSTANCE_H_

//#include "debug.h"
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



#define PORT 44024
#define GROUP 0xe0010101

class Network {
 public:
  int mySocket;
  struct sockaddr_in myAddr;
  struct sockaddr_in groupAddr;

  Network();
  virtual ~Network();
  struct sockaddr_in* resolveHost(register char *);
};
#endif /* NETWORKINSTANCE_H_ */

