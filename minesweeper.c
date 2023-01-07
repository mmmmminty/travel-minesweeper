#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define TILE_MINE 'M'
#define TILE_EMPTY ' '
#define TILE_UNKNOWN 'X'
#define TILE_BUFFER '#'
#define TILE_FLAG 'F'

#define COMMAND_FLAG 'F'
#define COMMAND_CHECK 'C'
#define COMMAND_CHEAT 'L'
#define COMMAND_QUIT 'Q'
#define COMMAND_REROLL 'R'
#define COMMAND_HINT 'H'
#define COMMAND_UNFLAG 'U'
#define COMMAND_INSTRUCTIONS 'I'

#define CURRENT_BOARD 'B'
#define CHEAT_BOARD 'C'

struct turn_info start_game(int height, int width, int mines);
struct game_board **init_board(int height, int width, int mines);
struct game_board **restart(int height, int width, int mines, int *correct, int *hint_count, int *placed_flags);
void assign_buffers(struct game_board **board, int height, int width);
void assign_mine_count(struct game_board **board, int height, int width);
void assign_mines(struct game_board **board, int height, int width, int mines);
int get_surrounding(int x, int y, struct game_board **board);

void print_board(struct game_board **board, int height, int width, char flag);
void print_instructions(void);

void reveal_adjacent(int x, int y, struct game_board **board, int height, int width);
void flag_tile(int x, int y, struct game_board **board, int height, int width, int *placed_flags);
void unflag_tile(int x, int y, struct game_board **board, int height, int width, int *placed_flags);
int check_win(struct game_board **board, int height, int width, int mines);
void end_turn(struct game_board **board, int height, int width, int mines, int placed_flags, int *correct);
bool check_mine(int x, int y, struct game_board **board);

int crimp(int num, int max);
void color(char color);

struct game_board {
    char tile;
    int mine_count;

    bool is_mine;
    bool is_buffer;
    bool is_flagged;
    bool is_found;
};

struct turn_info {
    struct game_board **board;
    int height;
    int width;
    int mines;

    int *correct;
    int *placed_flags;
    int *hint_count;

    int x;
    int y;
};

int main(void) {
    int height, width, mines;
    
    color('y');
    printf("\nEnter dimensions and mine count (x, y, mc): \n\n");
    color('d');
    scanf("%d, %d, %d", &height, &width, &mines);

    height = height + 2;
    width = width + 2;

    struct turn_info turn = start_game(height, width, mines);
    struct game_board **board = turn.board;

    char command, temp;
    int x, y;
    int placed_flags = 0, hint_count = 0;
    int correct = check_win(board, height, width, mines);

    print_instructions();
    end_turn(board, height, width, mines, placed_flags, &correct);

    while (scanf("%c", &command) != EOF) {
        switch (command) {
            case COMMAND_CHEAT:
                print_board(board, height, width, CHEAT_BOARD);
                printf("Enter Command: ");
                break;

            case COMMAND_CHECK:
                scanf(" %d,%d", &x, &y);

                system("clear");
                if (check_mine(crimp(x, height), crimp(y, height), board)) {
                    board = restart(height, width, mines, &correct, &hint_count, &placed_flags);
                    print_board(board, height, width, CHEAT_BOARD);
                } else {
                    reveal_adjacent(x, y, board, height, width);
                }
                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_FLAG:
                scanf(" %d,%d", &x, &y);

                if (placed_flags == mines) {
                    color('r');
                    printf("\nMax flags placed, not all mines flagged!\n");
                } else {
                    flag_tile(x, y, board, height, width, &placed_flags);
                }
                system("clear");
                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_UNFLAG:
                scanf(" %d,%d", &x, &y);
                unflag_tile(x, y, board, height, width, &placed_flags);
                system("clear");
                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_HINT:
                color('p');
                system("clear");

                if (hint_count == 3) {
                    printf("\n You have used all 3 hints, try not sucking.\n");
                } else {
                    hint_count++;
                    printf("\nHINT: You have correctly flagged %d out of %d mines.\n",
                    correct, mines);
                }

                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_REROLL:
                system("clear");
                board = restart(height, width, mines, &correct, &hint_count, &placed_flags);
                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_INSTRUCTIONS:
                system("clear");
                print_instructions();
                end_turn(board, height, width, mines, placed_flags, &correct);
                break;

            case COMMAND_QUIT:
                color('y');
                printf("\nKekw loser, can't even minesweep :P\n\n");
                color('d');
                exit(1);
                break;
        }
    }
    return 0;
}

struct turn_info start_game(int height, int width, int mines) {
    struct game_board **board = init_board(height, width, mines);
    struct turn_info turn;

    turn.board = board;
    turn.height = height;
    turn.width = width;

    turn.correct = NULL;
    turn.hint_count = NULL;
    turn.placed_flags = NULL;

    turn.x = 1;
    turn.y = 1;

    return turn;
}

struct game_board **init_board(int height, int width, int mines) {
    struct game_board **board = malloc(height * sizeof(struct game_board*));
    for (int i = 0; i < height; i++) {
        board[i] = malloc(width * sizeof(struct game_board));
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            board[i][j].tile = TILE_EMPTY;
            board[i][j].mine_count = 0;

            board[i][j].is_buffer = false;
            board[i][j].is_flagged = false;
            board[i][j].is_found = false;
            board[i][j].is_mine = false;
        }
    }

    assign_buffers(board, height, width);
    assign_mines(board, height, width, mines);
    assign_mine_count(board, height, width);

    return board;
}

struct game_board **restart(int height, int width, int mines, int *correct, int *hint_count, int *placed_flags) {
    *correct = 0;
    *hint_count = 0;
    *placed_flags = 0;
    
    return init_board(height, width, mines);
}

void assign_buffers(struct game_board **board, int height, int width) {
    for (int x = 0; x < height; x++) {
        board[x][0].tile = TILE_BUFFER;
        board[x][0].is_buffer = true;
    }

    for (int x = 0; x < height; x++) {
        board[x][width - 1].tile = TILE_BUFFER;
        board[x][width - 1].is_buffer = true;
    }

    for (int y = 0; y < width; y++) {
        board[0][y].tile = TILE_BUFFER;
        board[0][y].is_buffer = true;
    }

    for (int y = 0; y < width; y++) {
        board[height - 1][y].tile = TILE_BUFFER;
        board[height - 1][y].is_buffer = true;
    }

    return;
}

void assign_mine_count(struct game_board **board, int height, int width) {
    for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
            if (board[x][y].is_buffer) {
                board[x][y].mine_count = 9;
            } else {
                board[x][y].mine_count = get_surrounding(x, y, board);
                if (board[x][y].tile == TILE_EMPTY && board[x][y].mine_count > 0) {
                    board[x][y].tile = board[x][y].mine_count + 48;
                }
            }
        }
    }

    return;
}

void assign_mines(struct game_board **board, int height, int width, int mines) {
    int assigned = 0;

    while (assigned != mines) {
        int random_x = crimp(arc4random_uniform(height), height);
        int random_y = crimp(arc4random_uniform(width), width);

        if (board[random_x][random_y].is_buffer || board[random_x][random_y].is_mine) {
            continue;
        } else {
            board[random_x][random_y].tile = TILE_MINE;
            board[random_x][random_y].is_mine = true;
            assigned++;
        }
    }

    return;
}

void print_instructions(void) {
    color('g');
    printf("\nInstructions: \n");
    printf("\nC - Used to check a tile. (Example: C 3,4)");
    printf("\nF - Used to flag a potential mine. (Example: F 3,4)");
    printf("\nU - Used to unflag a potential mine. (Example: U 3,4)");
    printf("\nH - Confirms the amount of correctly flagged mines. (Example: H)");
    printf("\nR - Rerolls the current board. (Example: R)");
    printf("\nQ - Quits the game. (Example: Q)");
    printf("\nI - Shows these instructions. (Example: I)\n");
    color('d');

    return;
}

void print_board(struct game_board **board, int height, int width, char flag) {
    color('y');
    printf("\n  ");
    printf("  ");
    for (int i = 1; i < width - 1; i++) {
        printf("|%d", i % 10);
    }
    printf("  ");  
    printf("\n");

    for (int i = 0; i < height; i++) {
        color('y');
        if (i == 0 || i == height - 1) {
            printf("  ");
        } else {
            printf(" %d", i % 10);
        }
        color('d');

        for (int j = 0; j < width; j++) {
            switch (flag) {
                case CURRENT_BOARD:
                    if (!board[i][j].is_buffer && !board[i][j].is_found && !board[i][j].is_flagged) {
                        printf("|%c", TILE_UNKNOWN);

                    } else {
                        if (board[i][j].is_buffer) color('y');
                        if (board[i][j].is_found) color('c');
                        if (board[i][j].is_mine) color('r');
                        if (board[i][j].is_flagged) {
                            color('p');
                            printf("|%c", TILE_FLAG);
                        } else {
                            printf("|%c", board[i][j].tile);
                        } 
                    }
                    break;

                case CHEAT_BOARD:
                    if (board[i][j].is_buffer) color('y');
                    if (board[i][j].is_found) color('c');
                    if (board[i][j].is_mine) color('r');
                    if (board[i][j].is_flagged) {
                        color('p');
                        printf("|%c", TILE_FLAG);
                    } else {
                        printf("|%c", board[i][j].tile);
                    } 
                    break;
            }

            color('d');
        }

        color('y');
        if (i == 0 || i == height - 1) {
            printf("  ");
        } else {
            printf("|%d", i % 10);
        }
        printf("\n");
    }

    printf("  ");
    printf("  ");
    for (int i = 1; i < width - 1; i++) {
        printf("|%d", i % 10);
    }   
    printf("  ");
    printf("\n\n");
    color('d');

    return;
}

int get_surrounding(int x, int y, struct game_board **board) {
    int mine_count = 0;

    if (board[x+1][y].is_mine) {
        mine_count++;
    } 
    if (board[x+1][y+1].is_mine) {
        mine_count++;
    } 
    if (board[x+1][y-1].is_mine) {
        mine_count++;
    } 
    if (board[x][y-1].is_mine) {
        mine_count++;
    } 
    if (board[x][y+1].is_mine) {
        mine_count++;
    } 
    if (board[x-1][y].is_mine) {
        mine_count++;
    } 
    if (board[x-1][y+1].is_mine) {
        mine_count++;
    } 
    if (board[x-1][y-1].is_mine) {
        mine_count++;
    } 
    if (board[x][y].is_mine) {
        mine_count = 9;
    } 

    return mine_count;
}

void reveal_adjacent(int x, int y, struct game_board **board, int height, int width) {    
    if (x >= height || y >= width || x < 0 || y < 0) {
        return;
    } else if (board[x][y].is_found || board[x][y].is_buffer || board[x][y].is_mine) {
        return;
    } 

    if (board[x][y].mine_count == 0) {
        board[x][y].is_found = true;

        reveal_adjacent(x + 1, y, board, height, width);
        reveal_adjacent(x - 1, y, board, height, width);
        reveal_adjacent(x, y + 1, board, height, width);
        reveal_adjacent(x, y - 1, board, height, width);
    
    } else {
        board[x][y].is_found = true;

    }

    return;
}

int crimp(int num, int max) {
    if (num < 0) {
        num = 0;
    } else if (num > max) {
        num = max - 1;
    }

    return num;
}

void color(char color) {
    switch (color) {
        case 'y':
            printf("\033[1;33m");
            break;

        case 'r':
            printf("\033[1;31m");
            break;

        case 'g':
            printf("\033[1;32m");
            break;

        case 'b':
            printf("\033[1;34m");
            break;

        case 'p':
            printf("\033[1;35m");
            break;

        case 'c':
            printf("\033[1;36m");
            break;
        
        case 'd':
            printf("\033[0m");
            break;
    }

    return;
}

void flag_tile(int x, int y, struct game_board **board, int height, int width, int *placed_flags) {
    if (x >= height || y >= width || x < 0 || y < 0) {
        return;
    } else if (board[x][y].is_buffer || board[x][y].is_flagged) {
        return;
    } else {
        board[x][y].is_flagged = true;
        *placed_flags = *placed_flags + 1;
        return;
    }
}

void unflag_tile(int x, int y, struct game_board **board, int height, int width, int *placed_flags) {
    if (x >= height || y >= width || x < 0 || y < 0) {
        return;
    } else if (board[x][y].is_buffer || !board[x][y].is_flagged) {
        return;
    } else {
        board[x][y].is_flagged = false;
        *placed_flags = *placed_flags - 1;
        return;
    }
}

int check_win(struct game_board **board, int height, int width, int mines) {
    int correct = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j].is_mine && board[i][j].is_flagged) {
                correct++;
            }
        }
    }

    if (correct == mines) {
        color('g');
        printf("Congrats! You're not stupid :)\n\n");
        color('d');
        exit(1);
    }

    return correct;
}

void end_turn(struct game_board **board, int height, int width, int mines, int placed_flags, int *correct) {
    color('b');
    printf("\nSize: (%d,%d) | Mines: %d | Flags: %d | Placed: %d\n", 
    height - 2, width - 2, mines, mines - placed_flags, placed_flags);
    color('d');

    print_board(board, height, width, CURRENT_BOARD);
    *correct = check_win(board, height, width, mines);
    printf("Enter Command: ");
}

bool check_mine(int x, int y, struct game_board **board) {
    if (!board[x][y].is_mine) return false;

    char temp;

    color('r');
    printf("\nL Bozo that was a mine. Play again? (y/n) ");
    scanf("%c", &temp);

    if (temp == 'y') {
        return true;
    } else {
        exit(1);
    }
}