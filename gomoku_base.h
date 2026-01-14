#ifndef GOMOKU_BASE_H
#define GOMOKU_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 核心宏定义（保持不变，统一棋盘大小和数据类型）
#define BOARD_SIZE 15
#define ll long long

// 枚举类型（统一声明，避免重复定义，保持原有逻辑）
typedef enum {
    EMPTY,
    BLACK,
    WHITE
} CellState;

typedef enum {
    PLAYER_BLACK,
    PLAYER_WHITE
} Player;

typedef enum {
    MODE_PVP,  // 人人对战（保留你的原有功能）
    MODE_PVE   // 人机对战（可保留声明，也可删除，不影响编译）
} GameMode;

// 结构体类型（显式命名，避免类型不兼容，保持原有定义）
typedef struct Position {
    int row;
    int col;
} Position;

typedef struct GameState {
    CellState board[BOARD_SIZE][BOARD_SIZE];
    Player currentPlayer;
    Position lastMove;
    long long Score;
} GameState;

typedef struct {
    int row;
    int col;
    long long score;
} CandidatePosLL;

typedef struct {
    int row;
    int col;
    long long score;
} CandidatePos;
// 核心配置（保持不变，适配原有棋盘评估和棋库逻辑）
#define WIN_SCORE_LL 100000000000000LL
#define FORBIDDEN_SCORE_LL -100000000000000LL

// 关键修改1：移除 extern 全局变量声明（不再依赖 gomoku_train.c 的全局变量定义）
// 直接在头文件中定义全局变量（供 gomoku.c 直接使用，无需其他源文件）
extern long long chessWeights[6];

extern int dir[8][2];

extern int compare(const void *a, const void *b);

void sortCandidates(CandidatePos *candidates, int count);

int checktheSame(CellState a, Player b);

int in(int x, int y);

int getCell(GameState *gameState, int r, int c, CellState color);

int checkWin(GameState *gameState, int row, int col);
#endif 
