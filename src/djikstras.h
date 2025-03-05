#ifndef DIJKSTRAS_H
#define DIJKSTRAS_H

#include <stdbool.h>
#include "map.h"
#include "minheap.h"

void djikstras(mapObj map, loc pcLoc, bool tunnel);

#endif