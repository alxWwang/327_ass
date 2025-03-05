#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "djikstras.h"
#include "monsters.h"
#include "minheap.h"
#include "structs.h"
#include "utils.h"
#include "map.h"
#include "load_file.h"
#include "save_file.h"
#include <arpa/inet.h>


#ifdef _WIN32
    #define CLEAR_CMD "cls"
#else
    #define CLEAR_CMD "clear"
#endif

struct Node *pHead = NULL;

void clearScreen() {
    system(CLEAR_CMD);
}

mainMap generate(int gridSizeX, int gridSizeY, int min_count, int max_count){
    mapObj mainGrid = generateMainGrid(gridSizeX, gridSizeY);
    printf("%d, %d\n", gridSizeX, gridSizeY);
    int success_count = 0;
    mapEdges edges[max_count];
    roomInfo *room = NULL;

    int reset_count = 0;  // Track the number of resets
    int total_try = 0;    // Track the number of tries

    while (max_count != 0) {
        if (total_try >= 500) {
            if (success_count < min_count){
                mainGrid = generateMainGrid(gridSizeX, gridSizeY);
                reset_count++;
                total_try = 0;  // Reset try counter
                success_count = 0; // Reset success count
    
                if (reset_count >= 10) {
                    printf("Failed after 10 resets. Exiting...\n");
                    break;  // Stop execution after 10 resets
                }
            }else{
                break;
            }
            continue; // Restart the loop after reset
        }

        int width, height;
        width = 4 + rand() % 3;  // Random width between 4 and 6
        height = 3 + rand() % 3; // Random height between 6 and 8

        int posX = rand() % gridSizeX;
        int posY = rand() % gridSizeY;

        mapObj gen = generateGrid(width, height, '.');
        
        if (fitInGrid(mainGrid, gen, posX, posY)) {
            edges[success_count] = putInGrid(mainGrid, gen, posX, posY, success_count);
            insertRoomInfo(&room, initloc(posX, posY), width, height);
            success_count++;
            max_count--;
        }

        total_try++;  // Increment total attempts
    }

    if (success_count >= min_count) {
        printf("Successfully placed all required rooms.\n");
        printf("success count: %d\n", success_count);
        Node* randomEdges[success_count];
        for (int i = 0; i < success_count; i++) {
            randomEdges[i] = traverseNodes(edges[i].pHead, (rand() % edges[i].edgeNodeCt));
        }
        
        for(int i = 0; i<success_count; i++){
            int j = i;
            while (j==i){
                j = rand() % success_count;
            }
            connectRooms(mainGrid, 
                        initloc(randomEdges[i]->data.x, randomEdges[i]->data.y),
                        initloc(randomEdges[j]->data.x, randomEdges[j]->data.y)
                        );
        }

        mainMap mainStruct;
        mainStruct.mainMap = mainGrid;
        mainStruct.pcLoc = addPC(mainGrid);
        mainStruct.numRooms = success_count;
        mainStruct.up = createStairs(mainGrid,1, '<', true,0,0);
        mainStruct.down = createStairs(mainGrid, 1, '>',true,0,0);
        mainStruct.roomInfo = room;
        mainStruct.success = true;
        return mainStruct;

    }else{
        printf("Process failed.\n");
        printf("success count: %d\n", success_count);
        mainMap mainStruct;
        mainStruct.success = false;
        return mainStruct;
    }
    
}

void freeGrid(mapObj **grid, int rows) {
    for (int i = 0; i < rows; i++) {
        free(grid[i]);
    }
    free(grid);
}

bool handleInput(mainMap *main, bool bot){
    printf("select w/a/s/d to move, q to quit\n");
    char c;
    if (!bot){
        c = getchar();
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }else{
        char* line = "wasd";
        c = line[rand() % 4];
    }

    switch (c) {
        case 'w':
            if (main->pcLoc.y > 0) {
                movePC(main->mainMap, &main->pcLoc, main->pcLoc.y - 1, main->pcLoc.x);
            };
            break;
        case 's':
            if (main->pcLoc.y < main->mainMap.lenY - 1){
                movePC(main->mainMap, &main->pcLoc, main->pcLoc.y + 1, main->pcLoc.x);
            };
            break;
        case 'a':
            if (main->pcLoc.x > 0){
                movePC(main->mainMap, &main->pcLoc, main->pcLoc.y, main->pcLoc.x - 1);
            };
            break;
        case 'd':
            if (main->pcLoc.x < main->mainMap.lenX - 1) {
                movePC(main->mainMap, &main->pcLoc, main->pcLoc.y, main->pcLoc.x + 1);
            };
            break;
        case 'q':
            return true;  // signal quit
        default:
            // Ignore other keys
            break;
    }

    return false;
}


void runGameLoop(mainMap main) {
    int monstercount = 5;
    monster* ml[monstercount];
    MinHeap pq;

    pq.size = 0;
    pq.capacity = 50; // or however large you need
    pq.array = malloc(sizeof(HeapNode) * pq.capacity);

    HeapData data;
    data.eventData.time = 0;
    data.eventData.type = 1;
    insertMinHeap(&pq, data, EVENT, false);

    for (int i = 0; i < monstercount; i++) {
        unsigned int monsterType = rand() % 16;
        int randomX = 0;
        int randomY = 0;
        do {
            randomX = rand() % main.mainMap.lenX;
            randomY = rand() % main.mainMap.lenY;
        } while(main.mainMap.grid[randomY][randomX].repr != '.' || main.mainMap.grid[randomY][randomX].isMonster || main.mainMap.grid[randomY][randomX].isPC);
        ml[i] = addMonster(main.mainMap, true, monsterType, initloc(randomX, randomY), initloc(randomX, randomY));
        HeapData data;
        data.eventData.time = 0;
        data.eventData.mon = ml[i];
        data.eventData.type = 0;
        insertMinHeap(&pq, data, EVENT, false);
    }

    while (!isEmpty(&pq)) {
        clearScreen();
        event current = extractMin(&pq, false).data.eventData;
        int current_time = current.time;
        
        monster *m;
        int speed;

        printf("now is turn: %d\n", current_time);
        if (current.type == 0) {
            // Monster
            printf("---------------------------------------- BOT %c -------------------------------------------\n", current.mon->repr);
        } else {
            // PC
            printf("---------------------------------------- PC -------------------------------------------\n");
        }
        
        printGrid(main.mainMap.grid, main.mainMap.lenX, main.mainMap.lenY);
        printf("-----------------------------------------------------------------------------------------------------\n");
        if (current.type == 1) {
            if (handleInput(&main, true)) {
                break;
            };
            speed = 10;
            djikstras(main.mainMap, main.pcLoc, false);
            djikstras(main.mainMap, main.pcLoc, true);
            m = NULL;

            for (int i = 0; i < monstercount; i++) {
                if (inLineOfSight(main.mainMap, main.pcLoc, ml[i]->location)) {
                    ml[i]->lastSeenPC = main.pcLoc;
                }
                ml[i]->hasPath = false;
                ml[i]->hasVision = false;

                printf("look at line of sight of [%c]: ", ml[i]->repr);
                // if(getchar() == 'q'){
                //     break;
                // }
            }
            
        } else {
            m = current.mon;
            moveMonsterCombined(main, m, m->tuneling, m->erratic, m->telepathy, m->intelligence);
            speed = m->speed;
            printf("Press enter to move monster: ");
            // if(getchar() == 'q'){
            //     break;
            // }
        }

        int next_time = current_time + (1000 / speed);
        HeapData data;
        data.eventData.time = next_time;
        data.eventData.mon = m;
        data.eventData.type = current.type;
        insertMinHeap(&pq, data, EVENT, false);
        if (main.mainMap.grid[main.pcLoc.y][main.pcLoc.x].isMonster) {
            clearScreen();
            printf("Monster has killed the PC\n");
            printf("---------------------------------------- GG -------------------------------------------\n");
            printGrid(main.mainMap.grid, main.mainMap.lenX, main.mainMap.lenY);
            printf("----------------------------------------- GG ------------------------------------------------------------\n");
            break;
        }
        usleep(50000);
    }

    free(pq.array);
}

int main(int argc, char *argv[]){
    srand(time(NULL));
    int gridSizeX = 78;
    int gridSizeY = 19;
    int minimumCount = 6;
    int maximumCount = 6;
    
    bool save = false;
    bool load = false;

    char filename[50];
    char *home = getenv("HOME");
    char *loc = "/.rlg327/dungeon";
    strcpy(filename,home);
    strncat(filename, loc, sizeof(filename) - strlen(filename)-1);
    
    // char* filenameLoad = "01.rlg327";


    if (argc == 1){
        printf("Please select option --save or --load\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--save") == 0) {
            save = true;
        } else if (strcmp(argv[i], "--load") == 0) {
            load = true;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            return 0;
        }
    }

    mainMap main;
    main.success = false;

    if (load){
        loadMainMap(&main, filename);
        if(!main.success){
            printf("Loading Failed\n");
            return 0;
        }
    }
    if (save){
        main = generate(gridSizeX, gridSizeY, minimumCount, maximumCount );
        saveMainMap(&main, filename);
        if(!main.success){
            printf("Generate random dungeon failed\n");
            return 0;
        }
    }
    // bool erratic, bool tunnel, bool telepathic, bool intelligent
    
    djikstras(main.mainMap, main.pcLoc, false);
    djikstras(main.mainMap, main.pcLoc, true);

    runGameLoop(main);

    return 0;
}