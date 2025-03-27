#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "header/djikstras.h"
#include "header/monsters.h"
#include "header/minheap.h"
#include "header/structs.h"
#include "header/utils.h"
#include "header/map.h"
#include "header/load_file.h"
#include "header/save_file.h"
#include <arpa/inet.h>
#include <ncurses.h>


#ifdef _WIN32
    #define CLEAR_CMD "cls"
#else
    #define CLEAR_CMD "clear"
#endif

struct Node *pHead = NULL;

void clearScreen(WINDOW *win) {
    wrefresh(win);
}
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct LevelNode {
    mainMap level;
    monster* ml[50];                // Your dungeon level data
    struct LevelNode* prev;       // Pointer to the previous level (upstairs)
    struct LevelNode* next;       // Pointer to the next level (downstairs)
} LevelNode;

void moveMaps(LevelNode* current, int dir, int nummon);

// This function displays the monster list in its own window.
void displayMonsterList(monster **monsters, int monsterCount, loc pc) {
    // Define window size (you can adjust these values as needed)
    int list_win_height = 15;
    int list_win_width  = 50;
    int starty = (LINES - list_win_height) / 2;
    int startx = (COLS - list_win_width) / 2;
    
    WINDOW *list_win = newwin(list_win_height, list_win_width, starty, startx);
    keypad(list_win, TRUE);  // Enable arrow keys for this window
    // Draw a border around the window
    box(list_win, 0, 0);
    mvwprintw(list_win, 0, 2, " Monster List ");
    wrefresh(list_win);

    int offset = 0;
    int maxLines = list_win_height - 2; // accounting for border rows
    int ch;
    bool done = false;

    while (!done) {
        // Clear inner area and redraw border
        werase(list_win);
        box(list_win, 0, 0);
        mvwprintw(list_win, 0, 2, " Monster List ");
        
        // Display monsters from offset up to offset+maxLines
        for (int i = 0; i < maxLines && (i + offset) < monsterCount; i++) {
            monster *m = monsters[i + offset];
            int diffX = m->location.x - pc.x;
            int diffY = m->location.y - pc.y;
            char vertical[20] = "";
            char horizontal[20] = "";
            
            if (diffY < 0)
                sprintf(vertical, "%d north", -diffY);
            else if (diffY > 0)
                sprintf(vertical, "%d south", diffY);
            
            if (diffX < 0)
                sprintf(horizontal, "%d west", -diffX);
            else if (diffX > 0)
                sprintf(horizontal, "%d east", diffX);
            
            char line[100] = "";
            if (strlen(vertical) > 0 && strlen(horizontal) > 0)
                sprintf(line, "%c, %s and %s", m->repr, vertical, horizontal);
            else if (strlen(vertical) > 0)
                sprintf(line, "%c, %s", m->repr, vertical);
            else if (strlen(horizontal) > 0)
                sprintf(line, "%c, %s", m->repr, horizontal);
            else
                sprintf(line, "%c, at PC", m->repr);
            
            mvwprintw(list_win, i + 1, 1, "%s", line); 
        }
        wrefresh(list_win);
        
        // Get user input for scrolling or exit.
        ch = wgetch(list_win);
        switch (ch) {
            case KEY_UP:
                if (offset > 0)
                    offset--;
                break;
            case KEY_DOWN:
                if (offset < monsterCount - maxLines)
                    offset++;
                break;
            case 27:  // ESC key
                done = true;
                break;
            default:
                break;
        }
    }
    
    delwin(list_win);
    touchwin(stdscr);
    refresh();
}

void printGridMV(tile **grid, int gridSizeX, int gridSizeY, WINDOW *win){
    for (int i = 0; i< gridSizeY; i++){
        for (int j = 0; j < gridSizeX; j++){
            tile* g = &(grid[i][j]);
            if (g->isPC){
                wattron(win, COLOR_PAIR(1));
                mvwprintw(win, i+1,j+1,"@");
                wattroff(win, COLOR_PAIR(1));
            }
            else if (g->isMonster){
                wattron(win, COLOR_PAIR(2));
                mvwprintw(win, i+1,j+1,"%c",g->pMon->repr);
                wattroff(win, COLOR_PAIR(2));
            }
            else{
                mvwprintw(win, i+1,j+1,"%c", g->repr);
            }
        }
    }
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

bool handleInput(LevelNode* lNode, mainMap *main, bool bot, WINDOW *win, monster **mon, int monsterCount ) {
    char c;

    do {
        mvwprintw(win, 22, 1, "Select: y/k/u/h/j/l/b/n, m: MonsterList, Q: Quit");
        wrefresh(win);
        if (!bot) {
            c = wgetch(win);
        } else {
            const char *line = "ykuhljbn";
            c = line[rand() % 8];
        }
        // If user pressed 'm', display the monster list and loop again.
        if (c == 'm') {
            displayMonsterList(mon, monsterCount, main->pcLoc);
            // After closing the monster list window, reprint the grid and prompt again.
            printGridMV(main->mainMap.grid, main->mainMap.lenX, main->mainMap.lenY, win);
            wrefresh(win);
        }
        if (c == ERR){
            return false;
        }
    } while (c == 'm');  // Loop as long as the user presses 'm'

    int x = main->pcLoc.x;
    int y = main->pcLoc.y;
    int maxX = main->mainMap.lenX - 1;
    int maxY = main->mainMap.lenY - 1;

    switch (c) {
        // Move up-left
        case '7':
        case 'y':
        case 'q':
            if (y > 0 && x > 0) {
                movePC(main->mainMap, &main->pcLoc, y - 1, x - 1);
            }
            break;

        // Move up
        case '8':
        case 'k':
        case 'w':
            if (y > 0) {
                movePC(main->mainMap, &main->pcLoc, y - 1, x);
            }
            break;

        // Move up-right
        case '9':
        case 'u':
        case 'e':
            if (y > 0 && x < maxX) {
                movePC(main->mainMap, &main->pcLoc, y - 1, x + 1);
            }
            break;

        // Move right
        case '6':
        case 'l':
        case 'd':
            if (x < maxX) {
                movePC(main->mainMap, &main->pcLoc, y, x + 1);
            }
            break;

        // Move down-right
        case '3':
        case 'n':
        case 'x':
            if (y < maxY && x < maxX) {
                movePC(main->mainMap, &main->pcLoc, y + 1, x + 1);
            }
            break;

        // Move down
        case '2':
        case 'j':
        case 's':
            if (y < maxY) {
                movePC(main->mainMap, &main->pcLoc, y + 1, x);
            }
            break;

        // Move down-left
        case '1':
        case 'b':
        case 'z':
            if (y < maxY && x > 0) {
                movePC(main->mainMap, &main->pcLoc, y + 1, x - 1);
            }
            break;

        // Move left
        case '4':
        case 'h':
        case 'a':
            if (x > 0) {
                movePC(main->mainMap, &main->pcLoc, y, x - 1);
            }
            break;

        // Go down stairs
        case '>':
            // Only if on a '>' tile
            if (main->mainMap.grid[y][x].repr == '>') {
                werase(win);
                moveMaps(lNode, -1, monsterCount);
                // Logic to descend to new level
                // e.g., generate new dungeon, place PC, etc.
            }
            break;

        // Go up stairs
        case '<':
            // Only if on a '<' tile
            if (main->mainMap.grid[y][x].repr == '<') {
                werase(win);
                moveMaps(lNode, 1, monsterCount);
                // Logic to ascend to new level
            }
            break;

        // Rest for a turn
        // (no movement, but still uses one turn)
        case '5':
        case ' ':
        case '.':
            // Do nothing except consume turn
            break;
        // Quit the game
        case 'Q':
            return true;  // Signal that the user wants to quit

        default:
            // Ignore other keys
            break;
    }

    return false;
}

void runGameLoop(LevelNode *lNode, int ct, bool generate) {

    mainMap *mainState = &(lNode->level);

    djikstras(mainState->mainMap, mainState->pcLoc, false);
    djikstras(mainState->mainMap, mainState->pcLoc, true);

    initscr();
    cbreak();
    noecho();
    halfdelay(10);
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // PC color
        init_pair(2, COLOR_RED, COLOR_BLACK);      // Monster color
    }
    

    // Create a grid window that is sized based on the grid plus border,
    // and center it on the screen.
    int grid_win_height = mainState->mainMap.lenY + 2;
    int grid_win_width  = mainState->mainMap.lenX + 2;
    int starty = (LINES - grid_win_height) / 2;
    int startx = (COLS - grid_win_width) / 2;

    // Create the main window for status and input.
    WINDOW *win = newwin(1, grid_win_width, starty-1, startx);
    wrefresh(win);

    mvwprintw(win, 0, 0, "Welcome to the Dungeon! Use y, k, u, h, j, l, b, n for movement.");
    wrefresh(win);

    WINDOW *grid_win = newwin(grid_win_height, grid_win_width, starty, startx);
    box(grid_win, 0, 0);
    wrefresh(grid_win);

    int monstercount = ct;
    MinHeap pq;
    pq.size = 0;
    pq.capacity = 50;
    pq.array = malloc(sizeof(HeapNode) * pq.capacity);

    HeapData data;
    data.eventData.time = 0;
    data.eventData.type = 1; // PC turn.
    insertMinHeap(&pq, data, EVENT, false);
    
    if (generate){
        for (int i = 0; i < monstercount; i++) {
            unsigned int monsterType = rand() % 16;
            int randomX, randomY;
            do {
                randomX = rand() % mainState->mainMap.lenX;
                randomY = rand() % mainState->mainMap.lenY;
            } while (mainState->mainMap.grid[randomY][randomX].repr != '.' ||
                    mainState->mainMap.grid[randomY][randomX].isMonster ||
                    mainState->mainMap.grid[randomY][randomX].isPC);
            lNode->ml[i] = addMonster(mainState->mainMap, true, monsterType, initloc(randomX, randomY), initloc(randomX, randomY));
        }
    }
    monster **ml = lNode->ml;
    // Add monsters.
    for (int i = 0; i < monstercount; i++) {
        HeapData mData;
        mData.eventData.time = 0;
        mData.eventData.mon = ml[i];
        mData.eventData.type = 0;
        insertMinHeap(&pq, mData, EVENT, false);
    }
    
    int deadMonsterCount = 0;
    while (!isEmpty(&pq)) {
        werase(win);
        werase(grid_win);

        mvwprintw(win, 0, 0, "PC at (%d, %d)  -  Monsters remaining: %d",
                  mainState->pcLoc.x, mainState->pcLoc.y, monstercount - deadMonsterCount);
        wrefresh(win);

        event current = extractMin(&pq, false).data.eventData;
        int current_time = current.time;
        monster *m = NULL;
        int speed = 0;
        
        // // Display header in the main window.
        // if (current.type == 0) {
        //     mvwprintw(win, 0, 0, "-------------------------------------- BOT %c ---------------------------------------", current.mon->repr);
        // } else {
        //     mvwprintw(win, 0, 0, "--------------------------------------   PC   ---------------------------------------");
        // }
        
        // Display the grid in the grid window.
        printGridMV(mainState->mainMap.grid, mainState->mainMap.lenX, mainState->mainMap.lenY, grid_win);
        // mvwprintw(win, 23, 0, "------------------------------------------------------------------------------------");
        wrefresh(win);
        wrefresh(grid_win);
        
        if (current.type == 1) {  // PC turn.
            nodelay(grid_win, FALSE);
            if (handleInput(lNode, mainState, false, grid_win, ml, monstercount)) {
                exit(0);
                break;
            }
            nodelay(grid_win, TRUE);
            speed = 10;
            djikstras(mainState->mainMap, mainState->pcLoc, false);
            djikstras(mainState->mainMap, mainState->pcLoc, true);
            deadMonsterCount = 0;
            for (int i = 0; i < monstercount; i++) {
                if(ml[i]->alive == false){
                    deadMonsterCount++;
                    continue;
                }
                if (inLineOfSight(mainState->mainMap, mainState->pcLoc, ml[i]->location)) {
                    ml[i]->lastSeenPC = mainState->pcLoc;
                }
                ml[i]->hasPath = false;
                ml[i]->hasVision = false;
                // if(getchar() == 'q'){
                //     break;
                // }
            }


        } else {  // Monster turn.
            m = current.mon;
            if (!m->alive)
                continue;
            moveMonsterCombined(*mainState, m, m->tuneling, m->erratic, m->telepathy, m->intelligence);
            speed = m->speed;
        }
        
        int next_time = current_time + (1000 / speed);
        HeapData newData;
        newData.eventData.time = next_time;
        newData.eventData.mon = m;
        newData.eventData.type = current.type;
        insertMinHeap(&pq, newData, EVENT, false);
        
        if (mainState->mainMap.grid[mainState->pcLoc.y][mainState->pcLoc.x].isMonster || deadMonsterCount == monstercount) {
            werase(win);
            werase(grid_win);
            mvwprintw(win, 0, 0, "%s", deadMonsterCount == monstercount ? "PC wins!" : "Monster has killed the PC");
            printGridMV(mainState->mainMap.grid, mainState->mainMap.lenX, mainState->mainMap.lenY, grid_win);
            wrefresh(win);
            wrefresh(grid_win);
            nodelay(win, FALSE);
            wgetch(win);
            exit(0);
            break;
        }
    }
    free(pq.array);
    delwin(grid_win);
    delwin(win);
    endwin();
}

LevelNode* createLevelNode(mainMap levelData) {
    LevelNode* node = malloc(sizeof(LevelNode));
    if (!node) {
        // Handle allocation error
        return NULL;
    }
    node->level = levelData;
    for (int i = 0; i < 50; i++) {
        node->ml[i] = NULL;
    }
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void insertAfter(LevelNode* current, LevelNode* newNode) {
    if (!current || !newNode) return;
    newNode->next = current->next;
    newNode->prev = current;
    if (current->next)
        current->next->prev = newNode;
    current->next = newNode;
}

// Insert a new node before the current node
void insertBefore(LevelNode* current, LevelNode* newNode) {
    if (!current || !newNode) return;
    newNode->prev = current->prev;
    newNode->next = current;
    if (current->prev)
        current->prev->next = newNode;
    current->prev = newNode;
}

void moveMaps(LevelNode* current, int dir, int nummon){
    clear();
    refresh();
    if (dir == -1){
        //go down
        if (current->prev == NULL){
            insertBefore(current, createLevelNode(generate(79,19,6,6)));
            runGameLoop(current->prev, nummon, true);
        }else{
            runGameLoop(current->prev, nummon, false);
        }
    }else{
        if (current->next == NULL){
            insertAfter(current, createLevelNode(generate(79,19,6,6)));
            runGameLoop(current->next, nummon, true);
        }else{
            runGameLoop(current->next, nummon, false);
        }
    }
}

int main(int argc, char *argv[]){
    srand(time(NULL));
    int gridSizeX = 78;
    int gridSizeY = 19;
    int minimumCount = 6;
    int maximumCount = 6;
    int nummon = 10;
    
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
        } else if(strcmp(argv[i], "--nummon") == 0){
            if (i+1 >= argc){
                printf("Please provide number of monsters\n");
                return 0;
            }
            nummon = atoi(argv[i+1]); 
            i++;
        }
        else {
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
 
    LevelNode* mainLevel = createLevelNode(main);
    runGameLoop(mainLevel, nummon, true);

    return 0;
}