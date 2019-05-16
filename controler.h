#ifndef CONTROLER_H
#define CONTROLER_H

typedef int SOCKET;

struct client{
   SOCKET sock;
   struct vue * vue;
};

#endif
