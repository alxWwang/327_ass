#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>

/* Forward Declarations */
struct monster;
struct tile;
struct roomInfo;
struct Node;

/* Basic Types */

/* A simple 2D coordinate */
typedef struct {
    int x;
    int y;
} loc;

/* Linked List Node used for various lists */
typedef struct Node {
    loc data;
    struct Node *pNext;
} Node;

/* Stair List structure for up/down stairs */
typedef struct {
    int size;
    Node *pHead;
} stairList;

/* Map Edges: holds information about room boundaries */
typedef struct {
    int roomId;
    int edgeNodeCt;
    Node *pHead;
} mapEdges;

/* Room information in a linked list */
typedef struct roomInfo {
    loc location;
    int lenX;
    int lenY;
    struct roomInfo *pNext;
} roomInfo;

/* Map Object: a grid of tiles */
typedef struct {
    struct tile **grid;   // grid is a 2D array of tile pointers
    int lenX;
    int lenY;
} mapObj;

/* Monster: represents a creature in the game */
typedef struct monster {
    char repr;
    loc lastSeenPC;
    loc location;
    int speed;
    bool alive;
    int status;

    
    bool intelligence;
    bool telepathy;
    bool tuneling;
    bool erratic;

    bool hasPath;
    loc* path;
    int indexInPath;
    
    bool hasVision; 
    mapObj monsterVision;

} monster;

/* Tile: represents each cell in the map grid */
typedef struct tile {
    char repr;
    int hardness;
    bool isPC;
    loc location;
    int dist;
    int distTunnel;

    int monsterCount;
    bool isMonster;
    monster *pMon;
} tile;

/* Main Game Map structure: holds the complete game state */
typedef struct {
    mapObj mainMap;
    int numRooms;
    stairList up;
    stairList down;
    loc pcLoc;
    bool success;
    roomInfo *roomInfo;
} mainMap;

#endif // STRUCTS_H
