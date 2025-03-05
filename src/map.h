#ifndef MAP_H
#define MAP_H
#include <stdbool.h>
#include "structs.h"
#include "linked_list.h"

mapObj generateGrid(int gridSizeX, int gridSizeY, char type);
mapObj generateMainGrid(int gridSizeX, int gridSizeY);
mapEdges putInGrid(mapObj mainGrid,mapObj innerGrid, int atX,int  atY, int roomId);


bool fitInGrid(mapObj mainGrid, mapObj innerGrid, int atX, int atY);
void connectRooms(mapObj mainGrid, loc fromPoint, loc toPoint);
loc addPC(mapObj mainGrid);
void movePC(mapObj mainGrid,loc* currentLoc, int newY, int newX);
monster* addMonster(mapObj mainGrid, bool alive, int status, loc location, loc lastSeen);
stairList createStairs(mapObj mainGrid, int num, char type, bool random, int posX, int posY);
bool moveMonster(mapObj mainGrid, loc location, monster *mon);
bool inLineOfSight(mapObj mainGrid, loc fromPoint, loc toPoint);
// bool moveTelepathicIntelligent(mainMap main, monster *mon, bool tunnel, bool erratic);
// bool moveTelepathicNonIntelligent(mainMap main, monster *mon, bool tunnel, bool erratic);
// bool moveNonTelepathicIntelligent(mainMap main, monster *mon, bool tunnel, bool erratic);
// bool moveNonTelepathicNotIntelligent(mainMap main, monster *mon, bool tunnel, bool erratic);

bool moveMonsterCombined(mainMap main, monster *mon, bool tunnel, bool erratic, bool telepathic, bool intelligent);
#endif
