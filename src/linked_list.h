#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include "structs.h"

void insertLinkedList(Node **pHead, loc location);
Node* traverseNodes(Node *pHead, int atIndex);
void printAllNodes(Node *pHead);
void insertRoomInfo(roomInfo **pHead, loc location, int lenX, int lenY);
roomInfo* traverseRoomInfo(roomInfo *pHead, int atIndex);
void printAllRoomInfo(roomInfo *pHead);
void djikstras(mapObj map, loc pcLoc, bool tunnel);
#endif
