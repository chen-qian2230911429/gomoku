#include "gomoku_forbidden_move.h"

// 活三计数（判断是否为活三）


/* 提取某方向 9 个点（-4 ~ +4） */
void getLine(GameState *gameState, int row, int col,
             int dr, int dc, CellState color, int line[9]) {
    for (int i = -4; i <= 4; i++) {
        if (i == 0) {
            line[i + 4] = 1;  // 假设当前位置落子
        } else {
            int r = row + dr * i;
            int c = col + dc * i;
            line[i + 4] = getCell(gameState, r, c, color);
        }
    }
}

int countLiveThree(int line[9]) {
    int cnt = 0, empty = 0, block = 0;
    for(int i = 0;i <= 4;i++){
        if (line[i] == 1) cnt++;
        else if (line[i] == 0) empty++;
        else block++;
    }
    for (int i = 0; i <= 4; i++) {
        if(i!=0){
            if (line[i-1] == 1) cnt--;
            else if (line[i-1] == 0) empty--;
            else block--;
            if (line[i+4] == 1) cnt++;
            else if (line[i+4] == 0) empty++;
            else block++;
        }
        if (block) continue;
        if(line[i] != 0 && line[i+4] !=0) continue;
        //x__xx 型不算活三
        if (cnt == 3 && empty == 2) {
            int left  = (i - 1 >= 0) ? line[i - 1] : -1;
            int right = (i + 5 < 9) ? line[i + 5] : -1;
            if(line[i] == 0) left = 0;
            if(line[i+4] == 0) right = 0;
            if (left == 0 && right == 0)
                return 1;
        }
    }
    return 0;
}

// 检查是否有长连（超过5子，禁手）
int hasOverline(int line[9]) {
    int cnt = 0;
    for (int i = 0; i < 9; i++) {
        if (line[i] == 1) {
            cnt++;
            if (cnt >= 6) return 1;
        } else {
            cnt = 0;
        }
    }
    return 0;
}

// 冲四/活四计数（判断是否为冲四）
int countFour(int line[9]) {
    int cnt = 0, empty = 0, block = 0;
    for(int i = 0;i <= 4;i++){
        if (line[i] == 1) cnt++;
        else if (line[i] == 0) empty++;
        else block++;
    }
    for (int i = 0; i <= 4; i++) {
        if(i!=0){
            if (line[i-1] == 1) cnt--;
            else if (line[i-1] == 0) empty--;
            else block--;
            if (line[i+4] == 1) cnt++;
            else if (line[i+4] == 0) empty++;
            else block++;
        }
        if (block) continue;
        if (cnt == 4 && empty == 1) return 1;
    }
    return 0;
}

// 核心禁手判断（三三/四四/长连禁手）
int isValidMove(GameState *gameState, int row, int col, CellState color) {
    if (row < 0 || row >= BOARD_SIZE ||
        col < 0 || col >= BOARD_SIZE)
        return 0;
    if (gameState->board[row][col] != EMPTY)
        return 0;
    if (color == WHITE)  // 白棋无禁手
        return 1;
    
    int liveThreeCount = 0;  // 活三数量
    int fourCount = 0;       // 冲四/活四数量
    for (int d = 0; d < 4; d++) {
        int line[9];
        // 获取当前方向的9个点（-4~+4）
        getLine(gameState, row, col, dir[d][0], dir[d][1], color, line);

        // 特殊禁手：x0x0x 型（假活三，直接判禁手）
        if(line[3]==1&&line[2]==0&&line[1]==1&&line[5]==1&&line[6]==0&&line[7]==1) return 0;
        
        // 长连禁手（超过5子）
        if (hasOverline(line))
            return 0;
        
        // 统计冲四/活四（优先，一个方向只算最大的）
        int val = countFour(line);
        if(val) fourCount ++;
        else liveThreeCount += countLiveThree(line);
    }
    
    // 三三禁手（活三数量≥2）
    if (liveThreeCount >= 2)
        return 0;
    // 四四禁手（冲四/活四数量≥2）
    if (fourCount >= 2)
        return 0;
    
    return 1;
}