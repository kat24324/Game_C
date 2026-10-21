#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "map.h"

/* Helper: Create a deep copy of the map grid */
static char **copyMapGrid(char **src, int height, int width) {
    int i, j;
    char **dst;

    dst = (char **)malloc(height * sizeof(char *));
    if (dst == NULL) return NULL;

    for (i = 0; i < height; i++) {
        dst[i] = (char *)malloc(width * sizeof(char));
        if (dst[i] == NULL) {
            /* Clean up on failure */
            for (j = 0; j < i; j++) free(dst[j]);
            free(dst);
            return NULL;
        }
        for (j = 0; j < width; j++) {
            dst[i][j] = src[i][j];
        }
    }
    return dst;
}

/* Helper: Free a map snapshot */
static void freeMapSnapshot(char **grid, int height) {
    int i;
    if (grid == NULL) return;
    for (i = 0; i < height; i++) {
        free(grid[i]);
    }
    free(grid);
}

/* Push current game state onto linked list */
void pushState(Map *map, StateNode **head) {
    StateNode *newNode;

    newNode = (StateNode *)malloc(sizeof(StateNode));
    if (newNode == NULL) return;

    /* Copy simple fields */
    newNode->playerRow = map->playerRow;
    newNode->playerCol = map->playerCol;
    newNode->enemyRow = map->enemyRow;
    newNode->enemyCol = map->enemyCol;
    newNode->enemyDir = map->enemyDir;
    newNode->treasureTaken = map->treasureTaken;
    newNode->height = map->height;
    newNode->width = map->width;

    /* Deep copy the map grid */
    newNode->mapGrid = copyMapGrid(map->grid, map->height, map->width);
    if (newNode->mapGrid == NULL) {
        free(newNode);
        return;
    }

    /* Insert at head of list */
    newNode->next = *head;
    *head = newNode;
}

/* Pop (restore) previous game state from linked list */
int popState(Map *map, StateNode **head) {
    StateNode *oldNode;
    int i, j;

    if (*head == NULL) return 0;   /* Nothing to undo */

    oldNode = *head;
    *head = oldNode->next;

    /* Restore simple fields */
    map->playerRow = oldNode->playerRow;
    map->playerCol = oldNode->playerCol;
    map->enemyRow = oldNode->enemyRow;
    map->enemyCol = oldNode->enemyCol;
    map->enemyDir = oldNode->enemyDir;
    map->treasureTaken = oldNode->treasureTaken;

    /* Restore map grid */
    for (i = 0; i < map->height; i++) {
        for (j = 0; j < map->width; j++) {
            map->grid[i][j] = oldNode->mapGrid[i][j];
        }
    }

    /* Free the old node's snapshot */
    freeMapSnapshot(oldNode->mapGrid, oldNode->height);
    free(oldNode);

    return 1;
}

/* Free entire linked list (called at game end) */
void freeAllStates(StateNode **head) {
    while (*head != NULL) {
        StateNode *tmp = *head;
        *head = (*head)->next;
        freeMapSnapshot(tmp->mapGrid, tmp->height);
        free(tmp);
    }
}
