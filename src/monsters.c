#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "header/monsters.h"
#include "header/minheap.h"
#include "header/djikstras.h"
#include "header/map.h"

void printGridWithDistances(mapObj grid, monster *mon, bool tunnel, bool telepathic) {
    for (int i = 0; i < 19 + 2; i++) {
        for (int j = 0; j < 79 + 2; j++) {
            if (getDistDjikstras(grid.grid[i][j], tunnel) == INT_MAX) {
                printf(" . ");
            } else if (i == mon->location.y && j == mon->location.x) {
                printf("\033[31m%2d \033[0m", getDistDjikstras(grid.grid[i][j], tunnel));
            } else {
                printf("%2d ", getDistDjikstras(grid.grid[i][j], tunnel));
            }
        }
        printf("\n");
    }
}

monster* addMonster(mapObj mainGrid, bool alive, int status, loc location, loc lastSeen) {
    monster* monPtr = malloc(sizeof(monster));
    *monPtr = initmonster(alive, status, location, lastSeen, mainGrid);
    mainGrid.grid[location.y][location.x].isMonster = true;
    mainGrid.grid[location.y][location.x].pMon = monPtr;
    return monPtr; 
}

int getDistDjikstras(tile tiles, bool tunnel){
    if(tunnel){
        return tiles.distTunnel; 
    } else {
        return tiles.dist;       
    }
}

loc selectNextLocDjikstras(mapObj mainGrid, monster *mon, bool tunnel){
    tile **neighbors = getSurrounding(mon->location, mainGrid);
    int min = 256;
    loc location = mon->location;
    for (int i = 0; i < 8; i++){
        if (neighbors[i]->repr != '!'){
            int lowestDist = getDistDjikstras(*neighbors[i], tunnel);
            if (lowestDist < min && 
                !mainGrid.grid[neighbors[i]->location.y][neighbors[i]->location.x].isMonster) {
                min = lowestDist;
                location = neighbors[i]->location;
            }
        }
    }
    // free(neighbors);
    return location;
}

loc erraticLoc(mapObj mainGrid, monster *mon, bool tunnel){
    tile **neighbors = getSurrounding(mon->location, mainGrid);
    loc ret = mon->location;
    int r = 0;
    if (!tunnel){
        for (int i = 0; i < 22; i++){
            r = rand() % 8;
            if (neighbors[r]->hardness == 0){
                ret = neighbors[r]->location;
                // free(neighbors);
                return ret;
            }
        }
        // free(neighbors);
        return mon->location;
    } else {
        do {
            r = rand() % 8;
        } while (neighbors[r]->hardness == 255);
        ret = neighbors[r]->location;
        // free(neighbors);
        return ret;
    }
}

bool moveMonsterCombined(mainMap main, monster *mon, bool tunnel, bool erratic, bool telepathic, bool intelligent) {
    loc target;
    printf("Status: [tunnel: %s, erratic: %s, telepathic: %s, intelligent: %s]\n", 
           tunnel ? "true" : "false", 
           erratic ? "true" : "false", 
           telepathic ? "true" : "false", 
           intelligent ? "true" : "false");
    
    if (telepathic) {
        target = main.pcLoc;
    } else {
        target = mon->lastSeenPC;
    }

    if (mon->location.x == target.x && mon->location.y == target.y) {
        if (!telepathic) {
            mon->hasPath = false;
            mon->hasVision = false;
        }
        if(target.x == main.pcLoc.x && target.y == main.pcLoc.y){
            printf("The player is killed!");
        }
        return true;
    }

    loc bestLoc;
    if (erratic && rand() % 2 == 0) {
        bestLoc = erraticLoc(main.mainMap, mon, tunnel);
        if (!intelligent) {
            mon->hasPath = false;
            mon->hasVision = false;
        }
        printf("Moving erratically to: [%d, %d]\n", bestLoc.x, bestLoc.y);
    } else {
        if (intelligent) {
            if (telepathic) {
                bestLoc = selectNextLocDjikstras(main.mainMap, mon, tunnel);
                printf("Moving using Djikstra's (telepathic) to: [%d, %d]\n", bestLoc.x, bestLoc.y);
            } else {
                if(!mon->hasVision){
                    djikstras(mon->monsterVision, mon->lastSeenPC, tunnel);
                    mon->hasVision = true;
                }
                bestLoc = selectNextLocDjikstras(mon->monsterVision, mon, tunnel);
                printf("Moving using Djikstra's (non-telepathic) to: [%d, %d]\n", bestLoc.x, bestLoc.y);
            }
            
            // mapObj gridToPrint = telepathic ? main.mainMap : mon->monsterVision;
            // if(mon->hasVision || telepathic){
            //     printGridWithDistances(gridToPrint, mon, tunnel, telepathic);
            // }
    
        } else {
            if (!mon->hasPath) {
                mon->path = getStraightLine(main.mainMap, mon->location, target);
                mon->indexInPath = 0;
                mon->hasPath = true;
            }

            // mapObj tmp = copyMapObj(&main.mainMap);
            // int i =0;

            // while(mon->path[mon->indexInPath+i].x != target.x || mon->path[mon->indexInPath+i].y != target.y){
            //     tmp.grid[mon->path[mon->indexInPath + i].y][mon->path[mon->indexInPath + i].x].repr = 'x';
            //     i++;
            // }
            // if(mon->hasPath){
            //    printGrid(tmp.grid, tmp.lenX, tmp.lenY);
            // }

            if (!tunnel && main.mainMap.grid[mon->path[mon->indexInPath + 1].y]
                [mon->path[mon->indexInPath + 1].x].hardness == 0){
                return true;
            }

            bestLoc = mon->path[mon->indexInPath+1];
            if(moveMonster(main.mainMap, bestLoc, mon)){
                mon->indexInPath += 1;
            }
            
            printf("Moving using straight-line to: [%d, %d]\n", bestLoc.x, bestLoc.y);
            return false;
        }
    }

    moveMonster(main.mainMap, bestLoc, mon);
    return false;
}

bool moveMonster(mapObj mainGrid, loc location, monster *mon) {
    if (location.x < 0 || location.x >= mainGrid.lenX ||
        location.y < 0 || location.y >= mainGrid.lenY) {
        printf("Out of bounds: cannot move monster to [%d, %d]\n", location.x, location.y);
        return false;
    }

    if(mainGrid.grid[location.y][location.x].isMonster){
        return false;
    }

    int *cellHardness = &mainGrid.grid[location.y][location.x].hardness;
    printf("Hardness on tile [%d, %d] is %d\n", location.x, location.y, *cellHardness);
    
    if (*cellHardness > 85) {
        *cellHardness -= 85;
        return false;
    } else {
        mainGrid.grid[mon->location.y][mon->location.x].isMonster = false;
        mainGrid.grid[mon->location.y][mon->location.x].pMon = NULL;
        mainGrid.grid[location.y][location.x].isMonster = true;
        mainGrid.grid[location.y][location.x].pMon = mon;
        mon->location = location;
        if(mainGrid.grid[location.y][location.x].repr == ' '){
            mainGrid.grid[location.y][location.x].repr = '#';
        }
        return true;
    }
}