#ifndef UTILS_H
#define UTILS_H
#include "structs.h"

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
#endif
