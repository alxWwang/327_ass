#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "header/djikstras.h"
#include "header/utils.h"
#include "header/minheap.h"


void djikstras(mapObj map, loc pcLoc, bool tunnel) {
    MinHeap* heap = createMinHeap(map.lenX * map.lenY);
    setAllDistToInf(map, tunnel);
    setDist(&(map.grid[pcLoc.y][pcLoc.x]), tunnel, 0);
    HeapData data;
    data.tileData = map.grid[pcLoc.y][pcLoc.x];
    insertMinHeap(heap, data, TILE, tunnel);

    while (!isEmpty(heap)) {
        tile minTile = extractMin(heap, tunnel).data.tileData;
        tile **neighbors = getSurrounding(minTile.location, map);
    
        for (int i = 0; i < 8; i++) {
            tile* p = neighbors[i];
            if (p->hardness != 255 && isTunnel(tunnel, p)) {
                int newDist = getDist(&minTile, tunnel) + 1 + (p->hardness / 85);
                if (newDist < getDist(p, tunnel)) {
                    setDist(p, tunnel, newDist);
                    HeapData data = {.tileData = *p};
                    insertMinHeap(heap, data, TILE, tunnel);
                }
            }
        }
        //free(neighbors);
    }
}