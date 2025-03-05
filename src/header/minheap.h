#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdbool.h>
#include "structs.h"
#include "map.h"

typedef enum {
    TILE,
    EVENT
} HeapType;

typedef struct {
    int time;
    int type; // 0 for monster, 1 for PC
    monster* mon; 
} event;

typedef union {
    tile tileData;
    event eventData;
} HeapData;

typedef struct {
    HeapData data;
    HeapType type;
} HeapNode;

typedef struct {
    HeapNode *array;
    int size;
    int capacity;
} MinHeap;

MinHeap* createMinHeap(int cap);
void insertMinHeap(MinHeap *heap, HeapData data, HeapType type, bool tunel);
HeapNode extractMin(MinHeap* heap, bool tunnel);
bool isEmpty(MinHeap* heap);
#endif