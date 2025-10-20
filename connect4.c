// connect4.c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define ROWS 6
#define COLS 7


typedef enum { BOT_EASY = 1, BOT_MEDIUM = 2, BOT_HARD = 3 } BotLevel;

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
    const int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}}; 
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

static int choose_random_valid_col(const char b[ROWS][COLS]) {
    int valid[COLS];
    int n = 0;
    for (int c = 0; c < COLS; c++) {
        if (b[0][c] == '.') valid[n++] = c;
    }
    if (n == 0) return -1; // no moves
    int pick = rand() % n;
    return valid[pick];
}

static int choose_bot_move(char b[ROWS][COLS], char me, char opp, BotLevel lvl) {
    switch (lvl) {
        case BOT_EASY:
            return choose_random_valid_col(b);
        default:
            return choose_random_valid_col(b);
    }
}


static int read_human_col(char player) {
    char line[64];
    while (1) {
        printf("Player %c, choose column (1-7) or q to quit: ", player);
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) {
            puts("\nInput closed. Exiting.");
            return -2; 
        }
        if (line[0] == 'q' || line[0] == 'Q') return -2;

        char *end = NULL;
        long col = strtol(line, &end, 10);
        if (end == line || col < 1 || col > 7) {
            puts("Invalid input. Enter a number 1..7.");
            continue;
        }
        return (int)col - 1;
    }
}

int main(void) {
    char board[ROWS][COLS];
    srand((unsigned)time(NULL));   /* seed RNG */
    init_board(board);

    /* === Mode and difficulty selection BEFORE game starts === */
    int vs_bot = 0;
    BotLevel bot_level = BOT_EASY;

    puts("Select mode:");
    puts("  1) Human vs Human");
    puts("  2) Human vs Bot");
    printf("Enter 1 or 2: ");
    fflush(stdout);

    {
        char line[16];
        if (fgets(line, sizeof line, stdin) && line[0] == '2')
            vs_bot = 1;
    }

    if (vs_bot) {
        puts("Choose bot level:");
        puts("  1) Easy   (for losers)");
        printf("Enter 1/2/3: ");
        fflush(stdout);

        char line2[16];
        if (fgets(line2, sizeof line2, stdin)) {
            if (line2[0] == '2') bot_level = BOT_MEDIUM;
            else if (line2[0] == '3') bot_level = BOT_HARD;
        }
    }

    /* === Game setup === */
    char player = 'A';
    char bot = 'B';
    print_board(board);

    /* === Single main game loop === */
    while (1) {
        int col = -1, row = -1;

        if (vs_bot && player == bot) {
            /* Bot turn (currently EASY) */
            col = choose_bot_move(board, player,
            (player == 'A' ? 'B' : 'A'),
            bot_level);
            if (col < 0) { puts("No valid moves left."); break; }
            printf("Bot (%c) chooses column %d\n", player, col + 1);
        } else {
            /* Human turn */
            col = read_human_col(player);
            if (col == -2) { puts("Quit. Bye!"); break; }
        }

        row = drop_piece(board, col, player);
        if (row < 0) {
            puts("Column is full. Try another.");
            continue;
        }

        print_board(board);

        if (is_win(board, row, col, player)) {
            if (vs_bot && player == bot)
                printf("Bot (%c) WINS!\n", player);
            else
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
