#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"
#include "terminal.h"
#include "linkedlist.h"

int main(int argc, char *argv[]) {
    Map map;
    char input;
    int gameOver = 0;
    int win = 0;
    int i;
    StateNode *history = NULL;   /* Linked list head for UNDO feature */

    /* Initialize random number generator for enemy movement */
    srand(time(NULL));

    /* Check command line arguments */
    if (argc != 2) {
        printf("Usage: %s <map_file>\n", argv[0]);
        return 1;
    }

    /* Load map from file */
    if (loadMap(argv[1], &map) != 0) {
        return 1;
    }

    /* Main game loop */
    while (!gameOver) {
        printMap(&map);
        input = getch();

        if (input == 'q') {
            break;
        }
        else if (input == 'u') {
            /* UNDO: Restore previous state from linked list */
            if (popState(&map, &history)) {
                continue;   /* Refresh display */
            }
            /* If no state to restore, do nothing */
        }
        else if (input == 'w' || input == 's' || input == 'a' || input == 'd') {
            /* Save current state BEFORE moving (for UNDO) */
            pushState(&map, &history);

            /* Move player */
            movePlayer(&map, input, &gameOver, &win);

            /* If game not over, move enemy */
            if (!gameOver) {
                /* Enemy moves: 2 times normally, 3 times if furious (treasure taken) */
                int moves = map.treasureTaken ? 3 : 2;
                for (i = 0; i < moves; i++) {
                    moveEnemy(&map);
                    /* Check if enemy caught the player */
                    if (map.enemyRow == map.playerRow && map.enemyCol == map.playerCol) {
                        gameOver = 1;
                        win = 0;
                        break;
                    }
                }
            }
        }
        /* Other keys ignored */
    }

    /* Display final result */
    printMap(&map);
    if (win) {
        printf("You win!\n");
    } else if (gameOver && !win) {
        printf("You lose!\n");
    }
    getch();

    /* Clean up */
    freeAllStates(&history);
    freeMap(&map);

    return 0;
}
