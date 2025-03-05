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
bool saveFileMarker(FILE *fp) {
    const char *marker = "RLG327-S2025";
    if (fwrite(marker, 1, 12, fp) != 12) {
        fprintf(stderr, "Error: could not write file marker\n");
        fclose(fp);
        return false;
    }
    return true;
}

// 2) Read the 4-byte version (must be 0).
bool saveVersion(FILE *fp) {
    uint32_t version = 0;
    if (fwrite(&version, sizeof(version), 1, fp) != 1) {
        fprintf(stderr, "Error: could not write version\n");
        fclose(fp);
        return false;
    }
    return true;
}

// 3) Read the 4-byte file size (if you need it).
bool saveFileSize(FILE *fp, uint32_t fileSize) {
    if (fwrite(&fileSize, sizeof(fileSize), 1, fp) != 1) {
        fprintf(stderr, "Error: could not write file size placeholder\n");
        fclose(fp);
        return false;
    }
    return true;
}

// 4) Read the PC's position (two unsigned bytes).
bool savePCPosition(FILE *fp, mainMap *m) {
    if (fputc((unsigned char)m->pcLoc.x, fp) == EOF ||
        fputc((unsigned char)m->pcLoc.y, fp) == EOF) {
        fprintf(stderr, "Error: could not write PC position\n");
        fclose(fp);
        return false;
    }
    return true;
}

// 5) Read the hardness grid (assumed to be DUNGEON_X x DUNGEON_Y bytes).
bool saveHardnessGrid(FILE *fp, mainMap *m) {
    for (int y = 0; y < m->mainMap.lenY; y++) {
        for (int x = 0; x < m->mainMap.lenX; x++) {
            if (fputc((unsigned int)m->mainMap.grid[y][x].hardness, fp) == EOF){
                fprintf(stderr, "Error: could not write Hardness grid\n");
                fclose(fp);
                return false;
            }
        }
    }
    return true;
}

// 6) Read the number of rooms (16-bit).
bool saveNumRooms(FILE *fp, mainMap *m) {
    uint16_t numRooms = htons(m->numRooms);
    if (fwrite(&numRooms,sizeof(numRooms), 1, fp) != 1){
        fprintf(stderr, "Error: could not write Hardness grid\n");
        fclose(fp);
        return false;
    }
    return true;
}

// 7) Read each room's data (x, y, width, height) and insert into the map.
bool saveRooms(FILE *fp, mainMap *m) {
    for (int i = 0; i < m->numRooms; i++) {
        roomInfo *data = traverseRoomInfo(m->roomInfo,m->numRooms-i-1);
        if( fputc((unsigned int)data->location.x+1, fp) == EOF ||
            fputc((unsigned int)data->location.y+1, fp) == EOF ||
            fputc((unsigned int)data->lenX, fp) == EOF ||
            fputc((unsigned int)data->lenY, fp) == EOF){
                fprintf(stderr, "Error: could not write Rooms\n");
                fclose(fp);
                return false;
            }
    }
    return true;
}

// 8) Read the upward staircases.
bool saveStaircases(FILE *fp, mainMap *m, stairList *type, char typeChar) {
    uint16_t upCount = htons(m->up.size);
    if (fwrite(&upCount, sizeof(upCount), 1, fp) != 1) {
        fprintf(stderr, "Error: could not write up staircase count\n");
        fclose(fp);
        return false;
    }

    for (int i = 0; i < ntohs(upCount); i++) {
        loc data = traverseNodes(type->pHead, i)->data;
        if(fputc((unsigned int) data.x, fp) == EOF ||
        fputc((unsigned int) data.y, fp) == EOF){
            fprintf(stderr, "Error: could not write up staircase\n");
            fclose(fp);
            return false;
        }
    }
    return true;
}

bool deleteGeneratedFile(const char *filename) {
    if (remove(filename) != 0) {
        fprintf(stderr, "Error: Could not delete generated file\n");
        return false;
    }
    printf("File has failed to be saved\n");
    return true;
}


void saveMainMap(mainMap *m, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp || m->success == false) {
        fprintf(stderr, "Error: could not open %s\n", filename);
        return;
    }

    if (!saveFileMarker(fp))       { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveVersion(fp))          { fclose(fp); deleteGeneratedFile(filename); return; }


    // Record the position for file size, then write a placeholder.
    long fileSizeOffset = ftell(fp);
    uint32_t fileSizePlaceholder = 0;
    if (!saveFileSize(fp,fileSizePlaceholder)){ fclose(fp); deleteGeneratedFile(filename); return; }
    if (!savePCPosition(fp, m))    { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveHardnessGrid(fp, m))  { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveNumRooms(fp, m))      { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveRooms(fp, m))         { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveStaircases(fp, m, &m->up, '<')) { fclose(fp); deleteGeneratedFile(filename); return; }
    if (!saveStaircases(fp, m, &m->down, '>')) { fclose(fp); deleteGeneratedFile(filename); return; }


    long endPos = ftell(fp);
    uint32_t fileSize = (uint32_t)endPos;
    if (fseek(fp, fileSizeOffset, SEEK_SET) != 0) {
        fprintf(stderr, "Error: could not seek to file size placeholder\n");
        fclose(fp);
        deleteGeneratedFile(filename);
        return;
   }

   // Convert the file size to network byte order (if required).
   uint32_t netFileSize = htonl(fileSize);
   if (fwrite(&netFileSize, sizeof(netFileSize), 1, fp) != 1) {
        fprintf(stderr, "Error: could not update file size\n");
        fclose(fp);
        deleteGeneratedFile(filename);
        return;
   }
   fclose(fp);
}
