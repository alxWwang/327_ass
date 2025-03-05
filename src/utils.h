#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "structs.h"
#include "map.h"

tile inittile(char repr, int hardness, loc location);
mapObj initMapObj(tile **grid, int lenX, int lenY);
loc initloc(int x, int y);
mapEdges initEdges(int roomId, int edgeNodeCt, Node *pHead);

void printGrid(tile **grid, int gridSizeX, int gridSizeY);
void printGridHardness(tile **grid, int gridSizeX, int gridSizeY);

int custom_round(double value);
void loadMainMap(mainMap *m, const char * filename);
void printStatistics(mainMap main);
monster initmonster(bool alive, int status, loc location, loc lastSeenPC, mapObj monsterVision);

tile** getSurrounding(loc atLocation, mapObj mainMap);
mapObj copyMapObj(const mapObj *orig);

// --- Linked list functions for Node ---
void insertLinkedList(Node **pHead, loc location);
Node* traverseNodes(Node *pHead, int atIndex);
void printAllNodes(Node *pHead);

// --- RoomInfo linked list functions ---
void insertRoomInfo(roomInfo **pHead, loc location, int lenX, int lenY);
roomInfo* traverseRoomInfo(roomInfo *pHead, int atIndex);
void printAllRoomInfo(roomInfo *pHead);

// Distance utilities
int getDist(const tile *t, bool tunnel);
void setDist(tile *t, bool tunnel, int value);
void setAllDistToInf(mapObj mainmap, bool tunnel);
bool isTunnel(bool q, tile *curr);

#endif
