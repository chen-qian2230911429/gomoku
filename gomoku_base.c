#include "gomoku_base.h"

long long chessWeights[6] = {
    10000000LL,
    50000LL,
    5000LL,
    500LL,
    100LL,
    50LL
};

int dir[8][2] = {
    {0, 1},   {1, 1},   {1, 0},   {1, -1},  
    {0, -1},  {-1, -1}, {-1, 0},  {-1, 1}  
};



int compare(const void *a, const void *b) {
    // 将 void* 转换为 CandidatePos* 类型
    CandidatePos *pos1 = (CandidatePos*)a;
    CandidatePos *pos2 = (CandidatePos*)b;
    
    // 根据 score 从大到小排序
    if (pos1->score < pos2->score) {
        return 1;   // pos1 排在后面
    } else if (pos1->score > pos2->score) {
        return -1;  // pos1 排在前面
    } else {
        return 0;   // 相等
    }
}

void sortCandidates(CandidatePos *candidates, int count) { //排序
    qsort(candidates, count, sizeof(CandidatePos), compare);
}

int checktheSame(CellState a, Player b) { //相同
    if (a == BLACK && b == PLAYER_BLACK) return 1;
    if (a == WHITE && b == PLAYER_WHITE) return 1;
    return 0;
}

int in(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

int getCell(GameState *gameState, int r, int c, CellState color) {
    if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE)
        return -1;
    if (gameState->board[r][c] == color)
        return 1;
    if (gameState->board[r][c] == EMPTY)
        return 0;
    return -1;
}

int checkWin(GameState *gameState, int row, int col) {
    CellState color = gameState->board[row][col];
    if (color == EMPTY) return 0;
    Player targetPlayer = (color == BLACK) ? PLAYER_BLACK : PLAYER_WHITE;
    for (int i = 0; i < 4; i++) {
        int count = 1;
        int dx = dir[i][0], dy = dir[i][1];
        int nx = row + dx, ny = col + dy;
        while (in(nx, ny) && checktheSame(gameState->board[nx][ny], targetPlayer)) {
            count++;
            nx += dx;
            ny += dy;
        }
        dx = dir[i + 4][0], dy = dir[i + 4][1];
        nx = row + dx, ny = col + dy;
        while (in(nx, ny) && checktheSame(gameState->board[nx][ny], targetPlayer)) {
            count++;
            nx += dx;
            ny += dy;
        }
        if (count >= 5) {
            return (color == BLACK) ? 1 : 2;
        }
    }
    return 0;
}