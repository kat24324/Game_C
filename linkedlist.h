#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "map.h"

/* Linked list node for storing game state (UNDO feature) */
typedef struct StateNode {
    int playerRow;           /* Player position row */
    int playerCol;           /* Player position column */
    int enemyRow;            /* Enemy position row */
    int enemyCol;            /* Enemy position column */
    int enemyDir;            /* Enemy direction: 0=UP,1=RIGHT,2=DOWN,3=LEFT */
    int treasureTaken;       /* 1 if treasure collected, 0 otherwise */
    char **mapGrid;          /* Deep copy of the entire map */
    int height;              /* Map height (for cleanup) */
    int width;               /* Map width (for cleanup) */
    struct StateNode *next;  /* Pointer to next node in list */
} StateNode;

/* Save current game state to linked list (called BEFORE player moves) */
void pushState(Map *map, StateNode **head);

/* Restore previous game state from linked list (called on 'u') */
/* Returns 1 if successful, 0 if no state to restore */
int popState(Map *map, StateNode **head);

/* Free all nodes in linked list (called at game end) */
void freeAllStates(StateNode **head);

#endif
