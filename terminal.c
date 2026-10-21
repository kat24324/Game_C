#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "terminal.h"

void clearScreen(void) {
    /* ANSI escape sequence to clear screen and move cursor to home */
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}

char getch(void) {
    struct termios oldt, newt;
    char ch;

    /* Disable canonical mode and echo */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /* Read single character */
    ch = getchar();

    /* Restore original terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}
