#ifndef MONSTERS_H
#define MONSTERS_H

#include <stdbool.h>
#include "map.h"
#include "structs.h"
#include "utils.h"

monster* addMonster(mapObj mainGrid, bool alive, int status, loc location, loc lastSeen);
int getDistDjikstras(tile tiles, bool tunnel);
loc selectNextLocDjikstras(mapObj mainGrid, monster *mon, bool tunnel);
loc erraticLoc(mapObj mainGrid, monster *mon, bool tunnel);
bool moveMonsterCombined(mainMap main, monster *mon, bool tunnel, bool erratic, bool telepathic, bool intelligent);
bool moveMonster(mapObj mainGrid, loc location, monster *mon);

#endif