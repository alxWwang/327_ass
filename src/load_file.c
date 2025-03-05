#include <stdbool.h>
#include <endian.h>
#include <arpa/inet.h>
#include <stdio.h>    
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "linked_list.h"
#include "map.h"

// If you're fixed on 80x21 for the grid, define them
#define DUNGEON_X 80
#define DUNGEON_Y 21

// 1) Read the 12-byte semantic file marker.
bool loadFileMarker(FILE *fp) {
    char marker[13];
    size_t bytesRead = fread(marker, 1, 12, fp);
    if (bytesRead < 12) {
        fprintf(stderr, "Error: Could not read the 12-byte marker.\n");
        return false;
    }
    marker[12] = '\0';
    if (strncmp(marker, "RLG327-S2025", 12) != 0) {
        fprintf(stderr, "Error: File marker mismatch.\n");
        return false;
    }
    return true;
}

// 2) Read the 4-byte version (must be 0).
bool loadVersion(FILE *fp) {
    uint32_t version;
    size_t readCount = fread(&version, sizeof(version), 1, fp);
    if (readCount < 1) {
        fprintf(stderr, "Error: Could not read the version.\n");
        return false;
    }
    if (version != 0) {
        fprintf(stderr, "Error: Version mismatch. Expected 0, got %u\n", version);
        return false;
    }
    return true;
}

// 3) Read the 4-byte file size (if you need it).
bool loadFileSize(FILE *fp) {
    uint32_t fileSize;
    if (fread(&fileSize, sizeof(fileSize), 1, fp) != 1) {
        fprintf(stderr, "Error: Could not read file size.\n");
        return false;
    }
    // Optionally, process fileSize if required.
    return true;
}

// 4) Read the PC's position (two unsigned bytes).
bool loadPCPosition(FILE *fp, mainMap *m) {
    int c;
    c = fgetc(fp); unsigned char pcx = (unsigned char)c;
    c = fgetc(fp); unsigned char pcy = (unsigned char)c;
    m->pcLoc.x = pcx; m->pcLoc.y = pcy;
    return true;
}

// 5) Read the hardness grid (assumed to be DUNGEON_X x DUNGEON_Y bytes).
bool loadHardnessGrid(FILE *fp, mainMap *m) {
    m->mainMap.lenX = DUNGEON_X;
    m->mainMap.lenY = DUNGEON_Y;
    m->mainMap.grid = malloc(DUNGEON_Y * sizeof(tile *));
    for (int i = 0; i < DUNGEON_Y; i++) {
        m->mainMap.grid[i] = malloc(DUNGEON_X * sizeof(tile));
    }
    for (int y = 0; y < DUNGEON_Y; y++) {
        for (int x = 0; x < DUNGEON_X; x++) {
            int hardness = fgetc(fp);
            // Set the tile symbol based on the boundaries and hardness.
            if (y == 0 || y == DUNGEON_Y - 1) {
                m->mainMap.grid[y][x] = inittile('-', hardness, initloc(x,y));
            } else if (x == 0 || x == DUNGEON_X - 1) {
                m->mainMap.grid[y][x] = inittile('|', hardness, initloc(x,y));
            } else if (hardness == 0) {
                m->mainMap.grid[y][x] = inittile('#', hardness, initloc(x,y));
            } else {
                m->mainMap.grid[y][x] = inittile(' ', hardness, initloc(x,y));
            }
        }
    }
    return true;
}

// 6) Read the number of rooms (16-bit).
bool loadNumRooms(FILE *fp, mainMap *m) {
    uint16_t numRooms;
    size_t itemsread = fread(&numRooms, sizeof(numRooms), 1, fp);
    if (itemsread < 1) {
        fprintf(stderr, "Error: Could not read numRooms.\n");
        return false;
    }
    m->numRooms = ntohs(numRooms);
    return true;
}

// 7) Read each room's data (x, y, width, height) and insert into the map.
bool loadRooms(FILE *fp, mainMap *m) {
    roomInfo* roomHead = NULL;
    for (int i = 0; i < m->numRooms; i++) {
        int rx = fgetc(fp);
        int ry = fgetc(fp);
        int rw = fgetc(fp);
        int rh = fgetc(fp);
        insertRoomInfo(&roomHead, initloc(rx-1, ry-1), rw, rh);
        // Adjust coordinates if necessary (here we subtract one, for example).
        mapObj room = generateGrid(rw, rh, '.');
        putInGrid(m->mainMap, room, rx - 1, ry - 1, i);
    }
    m->roomInfo = roomHead;
    return true;
}

// 8) Read the upward staircases.
bool readUpStaircases(FILE *fp, mainMap *m, stairList *type, char typeChar) {
    uint16_t upCount;
    size_t itemsRead = fread(&upCount, sizeof(upCount), 1, fp);

    if (itemsRead < 1) {
        fprintf(stderr, "Error: Could not read staircase.\n");
        return false;
    }
    for (int i = 0; i < ntohs(upCount); i++) {
        unsigned int ux = fgetc(fp);
        unsigned int uy = fgetc(fp);
        *type = createStairs(m->mainMap,1, typeChar, false,ux,uy);
    }
    return true;
}

void loadMainMap(mainMap *m, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: could not open %s\n", filename);
        m->success = false;
        return;
    }

    // Initialize stair lists.
    m->up.size = 0;
    m->up.pHead = NULL;
    m->down.size = 0;
    m->down.pHead = NULL;

    if (!loadFileMarker(fp))      { fclose(fp); m->success = false; return; }
    if (!loadVersion(fp))         { fclose(fp); m->success = false; return; }
    if (!loadFileSize(fp))        { fclose(fp); m->success = false; return; }
    if (!loadPCPosition(fp, m))   { fclose(fp); m->success = false; return; }
    if (!loadHardnessGrid(fp, m)) { fclose(fp); m->success = false; return; }
    if (!loadNumRooms(fp, m))     { fclose(fp); m->success = false; return; }
    if (!loadRooms(fp, m))        { fclose(fp); m->success = false; return; }
    if (!readUpStaircases(fp, m, &m->up, '<')) { fclose(fp); m->success = false; return; }
    if (!readUpStaircases(fp, m, &m->down, '>')) { fclose(fp); m->success = false; return; }

    // Mark the PC's location in the grid.
    m->mainMap.grid[m->pcLoc.y][m->pcLoc.x].isPC = true;

    fclose(fp);
    m->success = true;
}
