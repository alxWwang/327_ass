#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include "map.h"
#include "structs.h"
#include "utils.h"
#include "linked_list.h"

mapObj generateGrid(int gridSizeX, int gridSizeY, char type){
    tile **grid = (tile **)malloc(gridSizeY * sizeof(tile *));
    for (int i = 0; i < gridSizeY; i++) {
        grid[i] = (tile *)malloc(gridSizeX * sizeof(tile));
    }
    for (int i = 0; i < gridSizeY; i++) {
        for (int j = 0; j < gridSizeX; j++) {
            grid[i][j] = inittile(type, 0, initloc(j,i));
        }
    }

    return initMapObj(grid, gridSizeX, gridSizeY);
}

mapObj generateMainGrid(int gridSizeX, int gridSizeY){
    gridSizeX+=2;
    gridSizeY+=2;
    tile **grid = (tile **)malloc(gridSizeY * sizeof(tile *));
    for (int i = 0; i < gridSizeY; i++) {
        grid[i] = (tile *)malloc(gridSizeX * sizeof(tile) );
    }
    for (int i = 0; i < gridSizeY; i++) {
        for (int j = 0; j < gridSizeX; j++) {
            if (i == 0 || i == gridSizeY-1){
                grid[i][j] = inittile('-', 255, initloc(j,i));
            }
            else if (j == 0 || j == gridSizeX-1 ){
                grid[i][j] = inittile('|', 255,initloc(j,i));
            }else{
                grid[i][j] = inittile(' ', (rand()% 254)+1, initloc(j,i));
            }   
        }
    }
    return initMapObj(grid, gridSizeX, gridSizeY);
}

mapEdges putInGrid(mapObj mainGrid,mapObj innerGrid, int atX,int  atY, int roomId){
    Node *pHead = NULL;
    int edgeCT = 0;
    for (int i = 0; i< innerGrid.lenY; i++){
        for (int j = 0; j< innerGrid.lenX ; j++){
            loc tLoc = mainGrid.grid[atY+i+1][atX+j+1].location;
            mainGrid.grid[atY+i+1][atX+j+1] = innerGrid.grid[i][j];
            mainGrid.grid[atY+i+1][atX+j+1].location = tLoc;
            if(i == 0 || i == innerGrid.lenY-1 || j == 0 || j == innerGrid.lenX-1){
                insertLinkedList(&pHead, initloc(atY+i+1,atX+j+1));
                edgeCT+=1;
            }
        }
    }
    return initEdges(roomId, edgeCT,pHead);;
}

bool fitInGrid(mapObj mainGrid, mapObj innerGrid, int atX, int atY){

    if (atY < 0 || atX < 0 || 
        (atY + innerGrid.lenY + 2) > mainGrid.lenY || 
        (atX + innerGrid.lenX + 2) > mainGrid.lenX) {
        return false;
    }

    for (int i = 0; i< innerGrid.lenY + 2; i++){
        for (int j = 0; j< innerGrid.lenX + 2 ; j++){
            char repr = mainGrid.grid[atY+i][atX+j].repr;
            if (repr != ' '){
                return false;
            }else{
            }
            
        }
    }
    return true;
}

loc* getStraightLine(mapObj mainGrid, loc fromPoint, loc toPoint){
    int x1 = fromPoint.x, y1 = fromPoint.y;
    int x2 = toPoint.x, y2 = toPoint.y;

    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    int max = abs(x2-x1) + abs(y2-y1)+1;
    loc *linePath = malloc(max * sizeof(loc));
    int i =0;
    while (true) {
        linePath[i] = initloc(x1,y1);;
        i++;

        if (x1 == x2 && y1 == y2) break;

        e2 = 2 * err;
        if (e2 <= dx) { 
            err += dx; 
            y1 += sy; 
        }
        if (e2 >= dy) { 
            err += dy; 
            x1 += sx; 
        }
    }
    //hello git
    return linePath;
}

void connectRooms(mapObj mainGrid, loc fromPoint, loc toPoint){
    loc* line = getStraightLine(mainGrid, fromPoint, toPoint);
    int i = 0;
    loc lastLoc = fromPoint;
    while(true){
        if(lastLoc.x == toPoint.x && lastLoc.y == toPoint.y){
            break;
        }
        lastLoc = initloc(line[i].x, line[i].y);
        if (mainGrid.grid[line[i].x][line[i].y].repr == ' '){
            mainGrid.grid[line[i].x][line[i].y] = inittile('#', 0, lastLoc);
        }
        
        i++;

    }
    free(line);
}

loc addStairs(mapObj mainGrid, char type, bool random , int posX, int posY){
    if(random == false){
        mainGrid.grid[posY][posX].repr = type;
        return initloc(posX, posY);
    }

    while(true){
        if(mainGrid.grid[posY][posX].repr == '#' || mainGrid.grid[posY][posX].repr == '.'){
            mainGrid.grid[posY][posX].repr = type;
            return initloc(posX, posY);
        }
        posX = rand() % mainGrid.lenX;
        posY = rand() % mainGrid.lenY;
    }
}

stairList createStairs(mapObj mainGrid, int num, char type, bool random, int posX, int posY){
    stairList lis;
    lis.size = num;
    Node* stairHead = NULL;
    for(int i = 0; i<num; i++){
        insertLinkedList(&stairHead, addStairs(mainGrid, type, random, posX, posY));
    }
    lis.pHead = stairHead;
    return lis;
}

loc addPC(mapObj mainGrid){
    int posX = 0;
    int posY = 0;

    while(true){
        if(mainGrid.grid[posY][posX].repr == '.'){
            mainGrid.grid[posY][posX].isPC = true;
            return initloc(posX, posY);
        }
        posX = rand() % mainGrid.lenX;
        posY = rand() % mainGrid.lenY;
    }
}

void movePC(mapObj mainGrid,loc* currentLoc, int newY, int newX){
    mainGrid.grid[currentLoc->y][currentLoc->x].isPC = false;
    currentLoc->y = newY; 
    currentLoc->x = newX;
    mainGrid.grid[currentLoc->y][currentLoc->x].isPC = true;
}

monster* addMonster(mapObj mainGrid, bool alive, int status, loc location, loc lastSeen){
    monster* monPtr = malloc(sizeof(monster));      
    *monPtr = initmonster(alive, status, location, lastSeen, mainGrid);
    mainGrid.grid[location.y][location.x].isMonster = true;
    mainGrid.grid[location.y][location.x].pMon = monPtr; 
    return monPtr; 
}

int getDistDjikstras(tile tiles, bool tunnel){
    if(tunnel){
        return tiles.distTunnel; // Use tunneling distance when tunneling is allowed.
    }else{
        return tiles.dist;       // Use normal distance when tunneling is not allowed.
    }
}

loc selectNextLocDjikstras(mapObj mainGrid, monster *mon, bool tunnel){
    tile **neighbors = getSurrounding(mon->location, mainGrid);
    int min = 256;
    loc location = mon->location;
    for (int i =0; i < 8; i++){
        int lowestDist = getDistDjikstras(*neighbors[i], tunnel);
        if( lowestDist < min && !mainGrid.grid[neighbors[i]->location.y][neighbors[i]->location.x].isMonster){
            min = lowestDist;
            location = neighbors[i]->location;
        }
    }
    free(neighbors);
    return location;
}

loc erraticLoc(mapObj mainGrid, monster *mon, bool tunnel){
    tile **neighbors = getSurrounding(mon->location, mainGrid);
    loc ret = mon->location;
    int r = 0;
    if (!tunnel){
        for (int i =0; i< 22; i++){
            r = rand()%8;
            if (neighbors[r]->hardness==0){
                ret = neighbors[r]->location;
                free(neighbors);
                return ret;
            }
        }
        free(neighbors);
        return mon->location;
    }else{
        ret =  neighbors[rand()%8]->location;
        free(neighbors);
        return ret;
    }
    
}

// This move function will work if both the player or monsterVision (targets) are reachable. or else it would be an infinite loop.
// This should be a non issue because the player and the monster is always reachable.

bool moveMonsterCombined(mainMap main, monster *mon, bool tunnel, bool erratic, bool telepathic, bool intelligent) {
    loc target;
    // Choose the target: telepathic monsters always know the PC's current location.
    if (telepathic) {
        target = main.pcLoc;
    } else {
        target = mon->lastSeenPC;
    }

    // If the monster is already at the target, handle accordingly.
    if (mon->location.x == target.x && mon->location.y == target.y) {
        // For non-telepathic monsters, reaching the last seen location resets the path.
        if (!telepathic) {
            mon->hasPath = false;
        }
        if(target.x == main.pcLoc.x && target.y == main.pcLoc.y){
            printf("The player is killed!");
        }
        return true;
    }

    loc bestLoc;

    // Erratic movement: half the time choose an erratic location.
    if (erratic && rand() % 2 == 0) {
        bestLoc = erraticLoc(main.mainMap, mon, tunnel);
        // For non-intelligent monsters, clear any precomputed path.
        if (!intelligent) {
            mon->hasPath = false;
        }
        printf("Moving erratically to: [%d, %d]\n", bestLoc.x, bestLoc.y);
    } else {
        if (intelligent) {
            if (telepathic) {
                // Intelligent, telepathic: use Djikstra's on the main map.
                bestLoc = selectNextLocDjikstras(main.mainMap, mon, tunnel);
                printf("Moving using Djikstra's (telepathic) to: [%d, %d]\n", bestLoc.x, bestLoc.y);
            } else {
                if(!mon->hasVision){
                    djikstras(mon->monsterVision, mon->lastSeenPC, tunnel );
                    mon->hasVision = true;
                }
                // Intelligent, non-telepathic: use Djikstra's on the monster's vision.
                bestLoc = selectNextLocDjikstras(mon->monsterVision, mon, tunnel);
                printf("Moving using Djikstra's (non-telepathic) to: [%d, %d]\n", bestLoc.x, bestLoc.y);
                if(mon->hasVision){
                    for (int i = 0; i< 19+2; i++){
                        for (int j = 0; j < 79+2; j++){
                            if (getDistDjikstras(mon->monsterVision.grid[i][j], tunnel) == INT_MAX){
                                printf(" . ");
                            }else{
                                printf("%2d ", getDistDjikstras(mon->monsterVision.grid[i][j], tunnel));
                            }
                        }
                        printf("\n");
                    }
                }
            }
        } else {
            // Non-intelligent: use a straight-line path.
            if (!mon->hasPath) {
                mon->path = getStraightLine(main.mainMap, mon->location, target);
                mon->indexInPath = 0;
                mon->hasPath = true;
            }
            // Check if the next cell is impassable when tunneling is off.
            if (!tunnel && main.mainMap.grid[mon->path[mon->indexInPath + 1].y]
                                            [mon->path[mon->indexInPath + 1].x].hardness != 0) {
                return true;
            }
            mapObj tmp = copyMapObj(&main.mainMap);
            int i =0;
            while(!tmp.grid[mon->path[mon->indexInPath + i].y][mon->path[mon->indexInPath + i].x].isPC){
                tmp.grid[mon->path[mon->indexInPath + i].y][mon->path[mon->indexInPath + i].x].repr = 'x';
                i++;
            }
            if(mon->hasPath){
                for (int i = 0; i< 21; i++){
                    for (int j = 0; j < 81; j++){
                        tile* g = &(tmp.grid[i][j]);
                        if (g->isPC){
                            printf("@");
                        }
                        else if (g->isMonster){
                            printf("%c",g->pMon->repr);
                        }
                        else{
                            printf("%c", g->repr);
                        }
                    }
                    printf("\n");
                }
            }
            bestLoc = mon->path[mon->indexInPath+1];
            if(moveMonster(main.mainMap, bestLoc, mon)){
                mon->indexInPath += 1;
            }
            printf("Moving using straight-line to: [%d, %d]\n", bestLoc.x, bestLoc.y);
            return false;
        }
    }

    if(main.mainMap.grid[bestLoc.y][bestLoc.x].isMonster){
        return true;
    }

    moveMonster(main.mainMap, bestLoc, mon);
    return false;
}

bool moveMonster(mapObj mainGrid, loc location, monster *mon) {

    if (location.x < 0 || location.x >= mainGrid.lenX ||
        location.y < 0 || location.y >= mainGrid.lenY)
    {
        printf("Out of bounds: cannot move monster to [%d, %d]\n", location.x, location.y);
        return false;
    }

    int *cellHardness = &mainGrid.grid[location.y][location.x].hardness;
    printf("Hardness on tile [%d, %d] is %d\n", location.x, location.y, *cellHardness);
    
    if (*cellHardness > 85) {
        *cellHardness -= 85;
        return false;
    } else {
        // Remove monster from its current position
        mainGrid.grid[mon->location.y][mon->location.x].isMonster = false;
        mainGrid.grid[mon->location.y][mon->location.x].pMon = NULL;
        // Place monster in new cell
        mainGrid.grid[location.y][location.x].isMonster = true;
        mainGrid.grid[location.y][location.x].pMon = mon;
        mon->location = location;
        return true;
    }
}
