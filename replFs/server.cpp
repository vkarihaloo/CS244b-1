/*
 * server.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */


#include "server.h"
int main(int argc, char **argv)
{
    if(argc!=7){
        ERROR("Invalid arguments.\n");
        exit(-1);
    }
    int port=atoi(argv[2]);
    std::string mount(argv[4]);
    int dropRate=atoi(argv[6]);
    ServerInstance S(port, mount, dropRate);
    S.run();

    return 0;
}




