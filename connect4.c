// connect4.c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define ROWS 6
#define COLS 7

static void init_board(char b[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            b[r][c] = '.';
}

static void print_board(const char b[ROWS][COLS]) {
    puts("");
    for (int r = 0; r < ROWS; r++) {
        printf("| ");
        for (int c = 0; c < COLS; c++)
            printf("%c ", b[r][c]);
        puts("|");
    }
    puts("+-----------------+");
    puts("  1 2 3 4 5 6 7");
    puts("");
}

static int in_bounds(int r, int c) {
    return r >= 0 && r < ROWS && c >= 0 && c < COLS;
}

static int drop_piece(char b[ROWS][COLS], int col, char p) {
    if (col < 0 || col >= COLS) return -1;
    for (int r = ROWS - 1; r >= 0; r--) {
        if (b[r][col] == '.') {
            b[r][col] = p;
            return r; // row where it landed
        }
    }
    return -1; // column full
}

static int count_dir(const char b[ROWS][COLS], int r, int c, int dr, int dc, char p) {
    int cnt = 0;
    int rr = r + dr, cc = c + dc;
    while (in_bounds(rr, cc) && b[rr][cc] == p) {
        cnt++; rr += dr; cc += dc;
    }
    return cnt;
}

static int is_win(const char b[ROWS][COLS], int r, int c, char p) {
    // Count both directions for each axis; need 4 in a row (incl. (r,c))
    const int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}}; // H, V, diag down-right, diag up-right
    for (int i = 0; i < 4; i++) {
        int dr = dirs[i][0], dc = dirs[i][1];
        int total = 1 + count_dir(b, r, c,  dr,  dc, p)
                      + count_dir(b, r, c, -dr, -dc, p);
        if (total >= 4) return 1;
    }
    return 0;
}

static int is_draw(const char b[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++)
        if (b[0][c] == '.') return 0;
    return 1;
}

int main(void) {
    char board[ROWS][COLS];
    init_board(board);

    char player = 'A';
    print_board(board);

    char line[64];
    while (1) {
        printf("Player %c, choose column (1-7) or q to quit: ", player);
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) {
            puts("\nInput closed. Exiting."); break;
        }
        if (line[0] == 'q' || line[0] == 'Q') {
            puts("Quit. Bye!"); break;
        }

        char *end = NULL;
        long col = strtol(line, &end, 10);
        if (end == line || col < 1 || col > 7) {
            puts("Invalid input. Enter a number 1..7.");
            continue;
        }

        int row = drop_piece(board, (int)col - 1, player);
        if (row < 0) {
            puts("Column is full. Try another.");
            continue;
        }

        print_board(board);

        if (is_win(board, row, (int)col - 1, player)) {
            printf("Player %c WINS!\n", player);
            break;
        }
        if (is_draw(board)) {
            puts("Draw. Board is full.");
            break;
        }
        player = (player == 'A') ? 'B' : 'A';
    }
    return 0;
}