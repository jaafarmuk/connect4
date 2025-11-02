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

static int landing_row(const char b[ROWS][COLS], int col) {
    if (col < 0 || col >= COLS || b[0][col] != '.') return -1;
    for (int r = ROWS-1; r >= 0; --r) if (b[r][col] == '.') return r;
    return -1;
}
static int sim_drop(char b[ROWS][COLS], int col, char p) {
    int r = landing_row(b, col);
    if (r >= 0) b[r][col] = p;
    return r;
}


static int wins_if_play(char b[ROWS][COLS], int col, char p) {
    int r = sim_drop(b, col, p);
    if (r < 0) return 0;
    int w = is_win(b, r, col, p);
    b[r][col] = '.';
    return w;
}


static int score_through(const char b[ROWS][COLS], int r, int c, char p) {
    const int dirs[4][2] = {{0,1},{1,0},{1,1},{1,-1}};
    int score = 0;
    for (int i = 0; i < 4; ++i) {
        int dr = dirs[i][0], dc = dirs[i][1];

        int left = 0, rr=r, cc=c;
        while (in_bounds(rr-dr, cc-dc) && b[rr-dr][cc-dc] == p) { rr-=dr; cc-=dc; ++left; }

        int right = 0; rr=r; cc=c;
        while (in_bounds(rr+dr, cc+dc) && b[rr+dr][cc+dc] == p) { rr+=dr; cc+=dc; ++right; }

        int total = 1 + left + right;


        int open = 0;
        if (in_bounds(r-(left+1)*dr, c-(left+1)*dc) && b[r-(left+1)*dr][c-(left+1)*dc] == '.') open++;
        if (in_bounds(r+(right+1)*dr, c+(right+1)*dc) && b[r+(right+1)*dr][c+(right+1)*dc] == '.') open++;
        if (total >= 3 && open > 0) score += 6;        // strong threat (xxx_)
        else if (total == 2 && open > 0) score += 3;   // grows lines
        else if (total == 1 && open > 0) score += 1;   // small value
    }
    return score;
}

/* evaluate playing column col for player me vs opp */
static int eval_move(char b[ROWS][COLS], int col, char me, char opp) {
    int r = sim_drop(b, col, me);
    if (r < 0) return -100000;           // illegal
    if (is_win(b, r, col, me)) { b[r][col] = '.'; return 100000; }

    int s = 0;

    s += 2 * (3 - abs(3 - col));         


    s += score_through(b, r, col, me);


    int danger = 0;
    for (int c2 = 0; c2 < COLS; ++c2)
        if (wins_if_play(b, c2, opp)) danger++;
    s -= 4 * danger;

    b[r][col] = '.';
    return s;
}


static int choose_bot_move(char b[ROWS][COLS], char me, char opp, BotLevel lvl) {
    if (lvl == BOT_EASY) return choose_random_valid_col(b);

    if (lvl == BOT_MEDIUM) {
        int best_col = -1, best_score = -1000000;

        /* 1) immediate win */
        for (int c = 0; c < COLS; ++c)
            if (wins_if_play(b, c, me)) return c;

        /* 2) immediate block: if opp can win somewhere next, play there */
        for (int c = 0; c < COLS; ++c)
            if (wins_if_play(b, c, opp)) return c;

        /* 3) heuristic over legal columns */
        for (int c = 0; c < COLS; ++c) {
            if (landing_row(b, c) < 0) continue;
            int sc = eval_move(b, c, me, opp);
            if (sc > best_score) { best_score = sc; best_col = c; }
        }
        return (best_col >= 0) ? best_col : choose_random_valid_col(b);
    }

    /* BOT_HARD placeholder -> use medium for now */
    return choose_random_valid_col(b);
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
    srand((unsigned)time(NULL));   
    init_board(board);


    int vs_bot = 0;
    BotLevel bot_level = BOT_EASY;
    char line[64];  

    puts("Select mode:");
    puts("  1) Human vs Human");
    puts("  2) Human vs Bot");
    while (1) {
        printf("Enter 1 or 2: ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) {
            puts("\nInput closed. Exiting.");
            return 0;
        }
        if (line[0] == '1' || line[0] == '2') {
            vs_bot = (line[0] == '2');
            break;
        }
        puts("Invalid input. Please type 1 or 2.");
    }

    if (vs_bot) {
        puts("Choose bot level:");
        puts("  1) Easy   (random valid)");
        puts("  2) Medium (placeholder)");
        puts("  3) Hard   (placeholder)");
        while (1) {
            printf("Enter 1, 2, or 3: ");
            fflush(stdout);
            if (!fgets(line, sizeof line, stdin)) {
                puts("\nInput closed. Exiting.");
                return 0;
            }
            if (line[0] == '1' || line[0] == '2' || line[0] == '3') {
                if (line[0] == '2') bot_level = BOT_MEDIUM;
                else if (line[0] == '3') bot_level = BOT_HARD;
                break;
            }
            puts("Invalid input. Please type 1, 2, or 3.");
        }
    }


    char player = 'A';
    char bot = 'B';
    print_board(board);


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
