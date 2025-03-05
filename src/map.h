#ifndef MAP_H
#define MAP_H
#include <stdbool.h>
#include "structs.h"

mapObj generateGrid(int gridSizeX, int gridSizeY, char type);
mapObj generateMainGrid(int gridSizeX, int gridSizeY);
mapEdges putInGrid(mapObj mainGrid,mapObj innerGrid, int atX,int  atY, int roomId);


bool fitInGrid(mapObj mainGrid, mapObj innerGrid, int atX, int atY);
void connectRooms(mapObj mainGrid, loc fromPoint, loc toPoint);
loc addPC(mapObj mainGrid);
void movePC(mapObj mainGrid,loc* currentLoc, int newY, int newX);

stairList createStairs(mapObj mainGrid, int num, char type, bool random, int posX, int posY);
bool inLineOfSight(mapObj mainGrid, loc fromPoint, loc toPoint);
loc* getStraightLine(mapObj mainGrid, loc fromPoint, loc toPoint);
#endif
