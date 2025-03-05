#include <string.h>
#include <stdio.h>    
#include <stdint.h>   
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "structs.h"
#include "utils.h"


char get_hex_representation(uint8_t characteristics) {
    // Ensure that only the lowest 4 bits are considered.
    characteristics &= 0x0F;
    return (characteristics < 10) ? '0' + characteristics : 'a' + (characteristics - 10);
}


mapObj copyMapObj(const mapObj *orig) {
    mapObj newMap;
    newMap.lenX = orig->lenX;
    newMap.lenY = orig->lenY;
    newMap.grid = malloc(sizeof(tile *) * orig->lenY);
    if (!newMap.grid) {
    }
    for (int i = 0; i < orig->lenY; i++) {
        newMap.grid[i] = malloc(sizeof(tile) * orig->lenX);
        if (!newMap.grid[i]) {
        }
        for (int j = 0; j < orig->lenX; j++) {
            newMap.grid[i][j] = orig->grid[i][j];
        }
    }
    return newMap;
}


tile inittile(char repr, int hardness, loc location){
    tile newTile;
    newTile.hardness = hardness;
    newTile.repr = repr;
    newTile.isPC = false;
    newTile.location = location;
    newTile.isMonster = false;
    newTile.pMon = NULL;
    newTile.dist = INT_MAX;
    newTile.distTunnel = INT_MAX;
    return newTile;
}

monster initmonster(bool alive, int status, loc location, loc lastSeenPC, mapObj monsterVision){
    #define FLAG_ERRATIC      (1 << 3) // 0b1000
    #define FLAG_TUNNELING    (1 << 2) // 0b0100
    #define FLAG_TELEPATHY    (1 << 1) // 0b0010
    #define FLAG_INTELLIGENCE (1 << 0) // 0b0001
    
    monster newMon;
    newMon.alive = alive;
    newMon.status = status;
    newMon.location = location;
    newMon.lastSeenPC = lastSeenPC;
    // newMon.speed = (rand()%16) + 5;
    newMon.speed = 5;
    newMon.hasPath = false;
    newMon.hasVision = false;
    newMon.indexInPath = 0;
    newMon.path = NULL;
    newMon.monsterVision = copyMapObj(&monsterVision);

    newMon.erratic      = (status & FLAG_ERRATIC) != 0;
    newMon.tuneling     = (status & FLAG_TUNNELING) != 0;
    newMon.telepathy    = (status & FLAG_TELEPATHY) != 0;
    newMon.intelligence = (status & FLAG_INTELLIGENCE) != 0;

    newMon.repr = get_hex_representation(status);
    
    // printf("Erratic: %s\n", newMon.erratic ? "true" : "false");
    // printf("Tunnel: %s\n", newMon.tuneling ? "true" : "false");
    // printf("Telepathic: %s\n", newMon.telepathy ? "true" : "false");
    // printf("Intelligent: %s\n", newMon.intelligence ? "true" : "false");

    return newMon;
}

mapObj initMapObj(tile **grid, int lenX, int lenY){
    mapObj obj;
    obj.grid = grid;
    obj.lenX = lenX;
    obj.lenY = lenY;
    return obj;
}

loc initloc(int x, int y){
    loc location;
    location.x = x;
    location.y = y;
    return location;
}

mapEdges initEdges(int roomId, int edgeNodeCt, Node *pHead){
    mapEdges edge;
    edge.roomId = roomId;
    edge.pHead = pHead;
    edge.edgeNodeCt = edgeNodeCt;
    return edge;
}

mainMap initMainMap(mapObj mainGrid, int numRooms, stairList up, stairList down, loc pcLoc ){
    mainMap main;
    main.mainMap = mainGrid;
    main.numRooms = numRooms;
    main.up = up;
    main.down = down;
    main.pcLoc = pcLoc;

    return main;
}



void printStatistics(mainMap main){
    printGrid(main.mainMap.grid, main.mainMap.lenX, main.mainMap.lenY);
    printGridHardness(main.mainMap.grid, main.mainMap.lenX, main.mainMap.lenY);
    printf("Player location:[%d, %d]\n", main.pcLoc.y, main.pcLoc.x);
    printf("Number of rooms: %d\n", main.numRooms);
    printf("Stairs up {%d}: ", main.up.size);
    printAllNodes(main.up.pHead);
    printf("Stairs down {%d}: ", main.down.size);
    printAllNodes(main.down.pHead);
    printf("Corner Locations:\n");
    printAllRoomInfo(main.roomInfo);
}

void printGrid(tile **grid, int gridSizeX, int gridSizeY){
    for (int i = 0; i< gridSizeY; i++){
        for (int j = 0; j < gridSizeX; j++){
            tile* g = &(grid[i][j]);
            if (g->isPC){
                printf("\033[32m@\033[0m");
            }
            else if (g->isMonster){
                printf("\033[31m%c\033[0m",g->pMon->repr);
            }
            else{
                printf("%c", g->repr);
            }
        }
        printf("\n");
    }
}

void printGridHardness(tile **grid, int gridSizeX, int gridSizeY){
    for (int i = 0; i< gridSizeY; i++){
        for (int j = 0; j < gridSizeX; j++){
            printf("[%-3d]", grid[i][j].hardness);
        }
        printf("\n");
    }
}

int custom_round(double value) {
    if (value >= 0) {
        return (int)(value + 0.5);  // Round up if the value is positive
    } else {
        return (int)(value - 0.5);  // Round down if the value is negative
    }
}

int** getGridHardness(tile **grid, int gridSizeX, int gridSizeY){
    int** hardness = (int**)malloc(gridSizeX * gridSizeY * sizeof(int*));
    for (int i = 0; i < gridSizeY; i++) {
        hardness[i] = (int *)malloc(gridSizeX * sizeof(int));
    }

    for (int i = 0; i< gridSizeY; i++){
        for (int j = 0; j < gridSizeX; j++){
            hardness[i][j] = grid[i][j].hardness;
        }
    }

    return hardness;
}

tile** getSurrounding(loc atLocation, mapObj mainMap){
    tile** neighbors = (tile**)malloc(8 * sizeof(tile*));
    int ct = 0;
    for (int i = -1; i<= 1; i ++){
        for (int j = -1; j <= 1; j++){
            if ((i != 0 || j != 0) && 
                (atLocation.y + i >= 0) && 
                (atLocation.x + j >= 0) && 
                (atLocation.x + j < mainMap.lenX) && 
                (atLocation.y + i < mainMap.lenY)) {
                neighbors[ct] = &mainMap.grid[atLocation.y + i][atLocation.x + j];
                ct+=1;
            }
        }
    }

    while (ct < 8) {
        tile *dummy = malloc(sizeof(*dummy));
        *dummy = inittile('!', 255, initloc(INT16_MAX, INT16_MAX));
        neighbors[ct] = dummy;
        ct++;
    }

    return neighbors;
}

/* ---------------------- Linked List Functions ---------------------- */

void insertLinkedList(Node **pHead, loc location) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return;
    }
    newNode->data = location;
    newNode->pNext = *pHead;
    *pHead = newNode;
}

Node* traverseNodes(Node *pHead, int atIndex) {
    Node *traverse = pHead;
    while (atIndex != 0 && traverse != NULL) {
        traverse = traverse->pNext;
        atIndex--;
    }
    return traverse;
}

void printAllNodes(Node *pHead) {
    Node *traverse = pHead;
    while (traverse != NULL) {
        printf("[%d, %d] ", traverse->data.y, traverse->data.x);
        traverse = traverse->pNext;
    }
    printf("\n");
}

/* ---------------------- RoomInfo Functions ---------------------- */

void insertRoomInfo(roomInfo **pHead, loc location, int lenX, int lenY) {
    roomInfo *newNode = (roomInfo *)malloc(sizeof(roomInfo));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return;
    }
    newNode->location = location;
    newNode->lenX = lenX;
    newNode->lenY = lenY;
    newNode->pNext = *pHead;
    *pHead = newNode;
}

roomInfo* traverseRoomInfo(roomInfo *pHead, int atIndex) {
    roomInfo *traverse = pHead;
    while (atIndex != 0 && traverse != NULL) {
        traverse = traverse->pNext;
        atIndex--;
    }
    return traverse;
}

void printAllRoomInfo(roomInfo *pHead) {
    roomInfo *traverse = pHead;
    while (traverse != NULL) {
        printf("loc: [%d, %d] dim: [%d, %d]\n",
               traverse->location.y, traverse->location.x,
               traverse->lenX, traverse->lenY);
        traverse = traverse->pNext;
    }
    printf("\n");
}

/* ------------------- Distance Utilities ------------------- */

int getDist(const tile *t, bool tunnel) {
    return tunnel ? t->distTunnel : t->dist;
}

void setDist(tile *t, bool tunnel, int value) {
    if (tunnel) {
        t->distTunnel = value;
    } else {
        t->dist = value;
    }
}

void setAllDistToInf(mapObj mainmap, bool tunnel) {
    for (int i = 0; i < mainmap.lenY; i++) {
        for (int j = 0; j < mainmap.lenX; j++) {
            setDist(&(mainmap.grid[i][j]), tunnel, INT_MAX);
        }
    }
}

bool isTunnel(bool q, tile *curr) {
    if (q) { 
        return true;
    }
    return curr->hardness == 0;
}

