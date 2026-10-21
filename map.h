#ifndef MAP_H
#define MAP_H

/* Direction codes for enemy */
#define DIR_UP    0
#define DIR_RIGHT 1
#define DIR_DOWN  2
#define DIR_LEFT  3

typedef struct {
    int rows;           /* Number of internal rows (without border) */
    int cols;           /* Number of internal columns (without border) */
    int height;         /* Total rows including border */
    int width;          /* Total columns including border */
    char **grid;        /* 2D character array for map display */
    int **wallFlag;     /* NEW: 1 = wall, 0 = walkable (for white background) */
    int playerRow;      /* Player's current row */
    int playerCol;      /* Player's current column */
    int treasureTaken;  /* 1 = treasure collected, 0 = not yet */
    int enemyRow;       /* Enemy's current row */
    int enemyCol;       /* Enemy's current column */
    int enemyDir;       /* Enemy's facing direction */
} Map;

/* Load map from file, return 0 on success, -1 on failure */
int loadMap(const char *filename, Map *map);

/* Print map with colors and clear screen */
void printMap(const Map *map);

/* Free all dynamically allocated memory in Map */
void freeMap(Map *map);

/* Move player, update game state. Returns 1 if move was valid */
int movePlayer(Map *map, char direction, int *gameOver, int *win);

/* Enemy movement (core AI algorithm) */
void moveEnemy(Map *map);

#endif
