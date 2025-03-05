#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "minheap.h"
#include "utils.h"

MinHeap* createMinHeap(int cap) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->array = (HeapNode*)malloc(cap * sizeof(HeapNode));
    heap->size = 0;
    heap->capacity = cap;
    return heap;
}

int getPriority(const HeapNode *node, bool tunnel) {
    if (node->type == TILE) {
        return getDist(&(node->data.tileData), tunnel); // Assuming non-tunnel for priority
    } else {
        return node->data.eventData.time;
    }
}

void insertMinHeap(MinHeap *heap, HeapData data, HeapType type, bool tunnel) {
    int i = heap->size++;
    heap->array[i].data = data;
    heap->array[i].type = type;

    while (i && getPriority(&(heap->array[i]), tunnel) < getPriority(&(heap->array[(i-1)/2]), tunnel)) {
        HeapNode temp = heap->array[i];
        heap->array[i] = heap->array[(i-1)/2];
        heap->array[(i-1)/2] = temp;
        i = (i-1)/2;
    }
}

HeapNode extractMin(MinHeap* heap, bool tunnel) {
    HeapNode root = heap->array[0];
    heap->array[0] = heap->array[--heap->size];

    int i = 0;
    while (2 * i + 1 < heap->size) {
        int left = 2 * i + 1, right = 2 * i + 2;
        int smallest = left;
        if (right < heap->size && getPriority(&(heap->array[right]), tunnel) < getPriority(&(heap->array[left]), tunnel))
            smallest = right;
        if (getPriority(&(heap->array[i]), tunnel) <= getPriority(&(heap->array[smallest]), tunnel))
            break;
        HeapNode temp = heap->array[i];
        heap->array[i] = heap->array[smallest];
        heap->array[smallest] = temp;
        i = smallest;
    }

    return root;
}

bool isEmpty(MinHeap* heap) {
    return heap->size == 0;
}

void printHeap(MinHeap *heap) {
    for (int i = 0; i < heap->size; i++) {
        if (heap->array[i].type == TILE) {
            printf("Tile: [%d, %d, %d]\n", heap->array[i].data.tileData.hardness, heap->array[i].data.tileData.location.x, heap->array[i].data.tileData.location.y);
        } else {
            printf("Event: [time: %d, type: %d]\n", heap->array[i].data.eventData.time, heap->array[i].data.eventData.type);
        }
    }
}