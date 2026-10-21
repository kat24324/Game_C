#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "terminal.h"
#include "color.h"

/* Helper: Check if a position is within map bounds */
static int isValidPosition(const Map *map, int row, int col) {
    return (row >= 0 && row < map->height && col >= 0 && col < map->width);
}

/* Load map from text file */
int loadMap(const char *filename, Map *map) {
    FILE *fp;
    int i, j, val;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: Cannot open file '%s'\n", filename);
        return -1;
    }

    /* Read map dimensions */
    if (fscanf(fp, "%d %d", &map->rows, &map->cols) != 2) {
        printf("ERROR: Invalid map format (first line)\n");
        fclose(fp);
        return -1;
    }

    /* Calculate total size with border */
    map->height = map->rows + 2;
    map->width  = map->cols + 2;

    /* Initialize player/enemy positions to -1 (not found) */
    map->playerRow = -1;
    map->playerCol = -1;
    map->enemyRow = -1;
    map->enemyCol = -1;
    map->enemyDir = DIR_LEFT;
    map->treasureTaken = 0;

    /* Allocate grid (2D char array) */
    map->grid = (char **)malloc(map->height * sizeof(char *));
    if (map->grid == NULL) {
        fclose(fp);
        return -1;
    }

    /* Allocate wallFlag (2D int array) - NEW for wall background */
    map->wallFlag = (int **)malloc(map->height * sizeof(int *));
    if (map->wallFlag == NULL) {
        free(map->grid);
        fclose(fp);
        return -1;
    }

    for (i = 0; i < map->height; i++) {
        map->grid[i] = (char *)malloc(map->width * sizeof(char));
        map->wallFlag[i] = (int *)calloc(map->width, sizeof(int));  /* Initialize to 0 */
        if (map->grid[i] == NULL || map->wallFlag[i] == NULL) {
            /* Cleanup on allocation failure */
            for (j = 0; j < i; j++) {
                free(map->grid[j]);
                free(map->wallFlag[j]);
            }
            free(map->grid);
            free(map->wallFlag);
            fclose(fp);
            return -1;
        }
    }

    /* Initialize borders and interior spaces */
    for (i = 0; i < map->height; i++) {
        for (j = 0; j < map->width; j++) {
            if (i == 0 || i == map->height - 1 || j == 0 || j == map->width - 1) {
                map->grid[i][j] = '#';          /* Border */
                map->wallFlag[i][j] = 1;        /* Border is also a wall */
            } else {
                map->grid[i][j] = ' ';          /* Default interior space */
                map->wallFlag[i][j] = 0;
            }
        }
    }

    /* Read interior data from file */
    for (i = 1; i <= map->rows; i++) {
        for (j = 1; j <= map->cols; j++) {
            if (fscanf(fp, "%d", &val) != 1) {
                printf("ERROR: Not enough data in map file at row %d\n", i);
                fclose(fp);
                return -1;
            }
            switch (val) {
                case 0:  /* Empty walkable path */
                    map->grid[i][j] = ' ';
                    map->wallFlag[i][j] = 0;
                    break;
                case 1:  /* Wall (white background) */
                    map->grid[i][j] = ' ';
                    map->wallFlag[i][j] = 1;   /* NEW: Mark as wall */
                    break;
                case 2:  /* Goal */
                    map->grid[i][j] = 'G';
                    map->wallFlag[i][j] = 0;
                    break;
                case 3:  /* Treasure */
                    map->grid[i][j] = 'T';
                    map->wallFlag[i][j] = 0;
                    break;
                case 4:  /* Player */
                    map->grid[i][j] = 'P';
                    map->wallFlag[i][j] = 0;
                    map->playerRow = i;
                    map->playerCol = j;
                    break;
                case 5:  /* Enemy (starts facing left) */
                    map->grid[i][j] = '<';
                    map->wallFlag[i][j] = 0;
                    map->enemyRow = i;
                    map->enemyCol = j;
                    map->enemyDir = DIR_LEFT;
                    break;
                default:
                    map->grid[i][j] = ' ';
                    map->wallFlag[i][j] = 0;
                    break;
            }
        }
    }

    fclose(fp);

    /* Validate that player and enemy were found */
    if (map->playerRow == -1 || map->playerCol == -1) {
        printf("ERROR: No player (4) found in map\n");
        return -1;
    }
    if (map->enemyRow == -1 || map->enemyCol == -1) {
        printf("ERROR: No enemy (5) found in map\n");
        return -1;
    }

    return 0;
}

/* Print map with colors and clear screen */
void printMap(const Map *map) {
    int i, j;
    char c;

    clearScreen();

    for (i = 0; i < map->height; i++) {
        for (j = 0; j < map->width; j++) {
            c = map->grid[i][j];

            if (c == 'P') {
                /* Player: blue foreground */
                printf("%s%c%s", COLOR_BLUE, c, COLOR_RESET);
            }
            else if (c == 'G') {
                /* Goal: green background */
                printf("%s%c%s", BG_GREEN, c, COLOR_RESET);
            }
            else if (c == 'T') {
                /* Treasure: yellow background */
                printf("%s%c%s", BG_YELLOW, c, COLOR_RESET);
            }
            else if (c == '<' || c == '^' || c == '>' || c == 'v') {
                /* Enemy: red foreground normally, red background when furious */
                if (map->treasureTaken == 0) {
                    printf("%s%c%s", COLOR_RED, c, COLOR_RESET);
                } else {
                    printf("%s%c%s", BG_RED_WHITE, c, COLOR_RESET);
                }
            }
            else if (c == '#') {
                /* Border: no color */
                printf("%c", c);
            }
            else if (c == ' ') {
                /* NEW: Check if this space is a wall (needs white background) */
                if (map->wallFlag != NULL && map->wallFlag[i][j]) {
                    printf("%s %s", BG_WHITE, COLOR_RESET);
                } else {
                    printf("%c", c);
                }
            }
            else {
                printf("%c", c);
            }
        }
        printf("\n");
    }
    printf("Use w/s/a/d to move, u to undo, q to quit.\n");
    fflush(stdout);
}

/* Free all allocated memory in Map */
void freeMap(Map *map) {
    int i;
    if (map->grid != NULL) {
        for (i = 0; i < map->height; i++) {
            free(map->grid[i]);
        }
        free(map->grid);
        map->grid = NULL;
    }
    /* NEW: Free wallFlag */
    if (map->wallFlag != NULL) {
        for (i = 0; i < map->height; i++) {
            free(map->wallFlag[i]);
        }
        free(map->wallFlag);
        map->wallFlag = NULL;
    }
}

/* Move player, handle treasure collection, win/loss conditions */
int movePlayer(Map *map, char direction, int *gameOver, int *win) {
    int newRow = map->playerRow;
    int newCol = map->playerCol;
    char targetChar;

    /* Calculate new position based on direction */
    switch (direction) {
        case 'w': newRow--; break;
        case 's': newRow++; break;
        case 'a': newCol--; break;
        case 'd': newCol++; break;
        default: return 0;
    }

    /* Check bounds */
    if (!isValidPosition(map, newRow, newCol)) {
        return 0;
    }

    targetChar = map->grid[newRow][newCol];

    /* Cannot walk into border or wall */
    if (targetChar == '#' || (map->wallFlag && map->wallFlag[newRow][newCol])) {
        return 0;
    }

    /* Cannot enter Goal without treasure */
    if (!map->treasureTaken && targetChar == 'G') {
        return 0;
    }

    /* Collect treasure */
    if (targetChar == 'T') {
        map->treasureTaken = 1;
        map->grid[newRow][newCol] = ' ';   /* Remove treasure from map */
    }

    /* Check win condition: reached Goal with treasure */
    if (map->treasureTaken && targetChar == 'G') {
        *win = 1;
        *gameOver = 1;
        return 1;
    }

    /* Check loss condition: stepped on enemy */
    if (targetChar == '<' || targetChar == '^' || targetChar == '>' || targetChar == 'v') {
        *gameOver = 1;
        *win = 0;
        return 1;
    }

    /* Perform the move */
    map->grid[map->playerRow][map->playerCol] = ' ';
    map->playerRow = newRow;
    map->playerCol = newCol;
    map->grid[newRow][newCol] = 'P';

    return 1;
}

/* Helper: Check if a cell is walkable for the enemy */
static int isEnemyWalkable(const Map *map, int row, int col) {

    if (!isValidPosition(map, row, col)) return 0;
    if (map->grid[row][col] == '#') return 0;
    /* Wall (white background) blocks enemy */
    if (map->wallFlag && map->wallFlag[row][col]) return 0;
    /* Treasure and Goal block enemy (treat as walls) */
    if (map->grid[row][col] == 'T' || map->grid[row][col] == 'G') return 0;

    return 1;
}

/* Helper: Get position in front of enemy */
static void getForwardPosition(int row, int col, int dir, int *newRow, int *newCol) {
    *newRow = row;
    *newCol = col;
    switch (dir) {
        case DIR_UP:    (*newRow)--; break;
        case DIR_RIGHT: (*newCol)++; break;
        case DIR_DOWN:  (*newRow)++; break;
        case DIR_LEFT:  (*newCol)--; break;
    }
}

/* Helper: Get position to the left of enemy */
static void getLeftPosition(int row, int col, int dir, int *newRow, int *newCol) {
    *newRow = row;
    *newCol = col;
    switch (dir) {
        case DIR_UP:    (*newCol)--; break;
        case DIR_RIGHT: (*newRow)--; break;
        case DIR_DOWN:  (*newCol)++; break;
        case DIR_LEFT:  (*newRow)++; break;
    }
}

/* Helper: Get position to the right of enemy */
static void getRightPosition(int row, int col, int dir, int *newRow, int *newCol) {
    *newRow = row;
    *newCol = col;
    switch (dir) {
        case DIR_UP:    (*newCol)++; break;
        case DIR_RIGHT: (*newRow)++; break;
        case DIR_DOWN:  (*newCol)--; break;
        case DIR_LEFT:  (*newRow)--; break;
    }
}

/* Enemy AI movement - core algorithm */
void moveEnemy(Map *map) {
    int forwardRow, forwardCol;
    int leftRow, leftCol;
    int rightRow, rightCol;
    int forwardWalkable, leftWalkable, rightWalkable;
    int newRow, newCol;
    int newDir;
    char enemyChar;
    int choice;

    /* Get positions for forward, left, and right */
    getForwardPosition(map->enemyRow, map->enemyCol, map->enemyDir, &forwardRow, &forwardCol);
    getLeftPosition(map->enemyRow, map->enemyCol, map->enemyDir, &leftRow, &leftCol);
    getRightPosition(map->enemyRow, map->enemyCol, map->enemyDir, &rightRow, &rightCol);

    /* Check which directions are walkable */
    forwardWalkable = isEnemyWalkable(map, forwardRow, forwardCol);
    leftWalkable = isEnemyWalkable(map, leftRow, leftCol);
    rightWalkable = isEnemyWalkable(map, rightRow, rightCol);

    /* Decision tree for enemy movement */
    if (forwardWalkable) {
        if (leftWalkable || rightWalkable) {
            /* Intersection - random choice */
            choice = rand() % 2;
            if (choice == 0 && leftWalkable) {
                /* Turn left */
                newDir = (map->enemyDir + 3) % 4;
                getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
                map->enemyDir = newDir;
            } else if (rightWalkable) {
                /* Turn right */
                newDir = (map->enemyDir + 1) % 4;
                getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
                map->enemyDir = newDir;
            } else {
                /* Only forward is possible */
                newRow = forwardRow;
                newCol = forwardCol;
            }
        } else {
            /* Only forward is open */
            newRow = forwardRow;
            newCol = forwardCol;
        }
    } else {
        /* Forward is blocked */
        if (leftWalkable && rightWalkable) {
            /* Both sides open - random choice */
            choice = rand() % 2;
            if (choice == 0) {
                newDir = (map->enemyDir + 3) % 4;
                getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
                map->enemyDir = newDir;
            } else {
                newDir = (map->enemyDir + 1) % 4;
                getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
                map->enemyDir = newDir;
            }
        } else if (leftWalkable) {
            /* Only left is open */
            newDir = (map->enemyDir + 3) % 4;
            getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
            map->enemyDir = newDir;
        } else if (rightWalkable) {
            /* Only right is open */
            newDir = (map->enemyDir + 1) % 4;
            getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
            map->enemyDir = newDir;
        } else {
            /* Dead end - turn around 180 degrees */
            newDir = (map->enemyDir + 2) % 4;
            getForwardPosition(map->enemyRow, map->enemyCol, newDir, &newRow, &newCol);
            map->enemyDir = newDir;
        }
    }

    /* Set enemy character based on new direction */
    switch (map->enemyDir) {
        case DIR_UP:    enemyChar = '^'; break;
        case DIR_RIGHT: enemyChar = '>'; break;
        case DIR_DOWN:  enemyChar = 'v'; break;
        default:        enemyChar = '<'; break;
    }

    /* Move enemy: clear old position, set new position */
    map->grid[map->enemyRow][map->enemyCol] = ' ';
    map->enemyRow = newRow;
    map->enemyCol = newCol;
    map->grid[newRow][newCol] = enemyChar;
}
