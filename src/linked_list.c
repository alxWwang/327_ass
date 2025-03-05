#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "linked_list.h"
#include "utils.h"

// Min-Heap
typedef struct {
    tile *array;
    int size;
    int capacity;
} MinHeap;


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

Node* traverseNodes(Node *pHead, int atIndex){
    Node *traverse = pHead;
    while(atIndex != 0){
        if (traverse->pNext != NULL){
            traverse = traverse->pNext;
            atIndex -=1;
        }else{
            break;
        }
    }
    return traverse;
}

void printAllNodes(Node *pHead){
    Node *traverse = pHead;
    while(traverse != NULL){
        printf("[%d, %d] ", traverse->data.y, traverse->data.x);
        traverse = traverse->pNext;
    }
    printf("\n");
}

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

roomInfo* traverseRoomInfo(roomInfo *pHead, int atIndex){
    roomInfo *traverse = pHead;
    while(atIndex != 0){
        if (traverse->pNext != NULL){
            traverse = traverse->pNext;
            atIndex -=1;
        }else{
            break;
        }
    }
    return traverse;
}

void printAllRoomInfo(roomInfo *pHead){
    roomInfo *traverse = pHead;
    while(traverse != NULL){
        printf("loc: [%d, %d] dim: [%d, %d]\n", traverse->location.y, traverse->location.x, traverse->lenX, traverse->lenY);
        traverse = traverse->pNext;
    }
    printf("\n");
}

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

void setAllDistToInf(mapObj mainmap, bool tunnel){
    for(int i = 0; i < mainmap.lenY; i++){
        for(int j =0; j<mainmap.lenX; j++){
            setDist(&(mainmap.grid[i][j]), tunnel, INT_MAX);
        }
    }
}


MinHeap* createMinHeap(int cap){
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->array = (tile*)malloc(cap * sizeof(tile));
    heap->size = 0;
    heap->capacity = cap;
    return heap;
}

void insertMinHeap(MinHeap *heap, tile obj, bool tunnel){
    int i = heap->size++;
    heap->array[i] = obj;

    while( i && getDist(&(heap->array[i]), tunnel) < getDist(&(heap->array[(i-1)/2]), tunnel)){
        tile temp = heap->array[i];
        heap->array[i] = heap->array[(i-1)/2];
        heap->array[(i-1)/2] = temp;
        i = (i-1)/2;
    }
}

tile extractMin(MinHeap* heap, bool tunnel) {
    tile root = heap->array[0];
    heap->array[0] = heap->array[--heap->size];

    int i = 0;
    while (2 * i + 1 < heap->size) {
        int left = 2 * i + 1, right = 2 * i + 2;
        int smallest = left;
        if (right < heap->size && getDist(&(heap->array[right]), tunnel) < getDist(&(heap->array[left]), tunnel))
            smallest = right;
        if (getDist(&(heap->array[i]), tunnel) <= getDist(&(heap->array[smallest]), tunnel))
            break;
        tile temp = heap->array[i];
        heap->array[i] = heap->array[smallest];
        heap->array[smallest] = temp;
        i = smallest;
    }

    return root;
}

bool isEmpty(MinHeap* heap) {
    return heap->size == 0;
}

void printHeap(MinHeap *heap){
    for (int i=0; i< heap->size; i++){
        printf("[%d, %d, %d]\n", heap->array[i].hardness, heap->array[i].location.x, heap->array[i].location.y);
    }
}

bool isTunnel(bool q, tile *curr){
    if (q){return true;}
    return curr->hardness == 0;
}

void djikstras(mapObj map, loc pcLoc, bool tunnel){
    MinHeap* heap = createMinHeap(map.lenX*map.lenY);
    setAllDistToInf(map, tunnel);
    setDist(&(map.grid[pcLoc.y][pcLoc.x]), tunnel, 0);
    insertMinHeap(heap, map.grid[pcLoc.y][pcLoc.x], tunnel);

    while(!isEmpty(heap)){
        tile minTile = extractMin(heap, tunnel);
        tile **neighbors = getSurrounding(minTile.location, map);
    
        for (int i = 0; i< 8; i++){
            tile* p = neighbors[i];
            if (neighbors[i]->hardness != 255 && isTunnel(tunnel, neighbors[i])){
                int newDist = getDist(&minTile, tunnel) + 1 + (neighbors[i]->hardness / 85);
                if(newDist < getDist(neighbors[i],tunnel)){
                    setDist(neighbors[i], tunnel, newDist);
                    insertMinHeap(heap, *neighbors[i], tunnel);
                }
            }
        }
        free(neighbors);
    }
}
