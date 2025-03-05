#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "monsters.h"
#include "minheap.h"
#include "djikstras.h"
#include "map.h"
#include "structs.h"
#include "utils.h"

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
                insertLinkedList(&pHead, initloc(atX+j+1, atY+i+1));
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

loc* getStraightLine(mapObj mainGrid, loc fromPoint, loc toPoint) {
    int x1 = fromPoint.x, y1 = fromPoint.y;
    int x2 = toPoint.x,   y2 = toPoint.y;

    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    int max = abs(x2 - x1) + abs(y2 - y1) + 1;
    loc *linePath = malloc(max * sizeof(loc));
    int i = 0;
    
    while (true) {
        linePath[i] = initloc(x1, y1);
        i++;

        if (x1 == x2 && y1 == y2) break;

        e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy; 
            x1 += sx; 
        }
        if (e2 <= dx) { 
            err += dx; 
            y1 += sy; 
        }
    }

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
        lastLoc = line[i];
        
        if (mainGrid.grid[line[i].y][line[i].x].repr == ' ') {
            mainGrid.grid[line[i].y][line[i].x] = inittile('#', 0, lastLoc);
        }        
        i++;

    }
    // free(line);
}

bool inLineOfSight(mapObj mainGrid, loc fromPoint, loc toPoint) {
    loc* line = getStraightLine(mainGrid, fromPoint, toPoint);
    int i = 0;
    loc lastLoc = fromPoint;
    while (true) {
        if (lastLoc.x == toPoint.x && lastLoc.y == toPoint.y) {
            free(line);
            return true;
        }
        lastLoc = line[i];
        
        if (mainGrid.grid[line[i].y][line[i].x].hardness != 0) {
            free(line);
            return false;
        }
        i++;
    }
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

    if (mainGrid.grid[newY][newX].hardness == 255){
        return;
    }
    mainGrid.grid[currentLoc->y][currentLoc->x].isPC = false;
    currentLoc->y = newY; 
    currentLoc->x = newX;
    mainGrid.grid[currentLoc->y][currentLoc->x].isPC = true;
    if (mainGrid.grid[currentLoc->y][currentLoc->x].repr == ' '){
        mainGrid.grid[currentLoc->y][currentLoc->x].repr = '#';
        mainGrid.grid[currentLoc->y][currentLoc->x].hardness = 0;
    }
}