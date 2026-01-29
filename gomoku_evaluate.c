#include "gomoku_evaluate.h"
#include <stdio.h>        // 调试printf（如需保留可删）


// 断活三判断（假活三）
int isBrokenLive3(GameState *gameState,int row, int col,int d, CellState color) {
    int dx = dir[d][0], dy = dir[d][1];
    // 模式一：X X _ X
    int x1 = row + dx,     y1 = col + dy;
    int x2 = row + 2*dx,   y2 = col + 2*dy;
    int x3 = row + 3*dx,   y3 = col + 3*dy;

    if (in(x1,y1)&&in(x2,y2)&&in(x3,y3)) {
        if (gameState->board[x1][y1]==color &&
            gameState->board[x2][y2]==EMPTY &&
            gameState->board[x3][y3]==color) {
            int lx = row - dx, ly = col - dy;
            int rx = x3 + dx,  ry = y3 + dy;
            if ((in(lx,ly)&&gameState->board[lx][ly]==EMPTY) &&
                (in(rx,ry)&&gameState->board[rx][ry]==EMPTY)) {
                return 1;
            }
        }
    }
    if (in(x1,y1)&&in(x2,y2)&&in(x3,y3)) {
        if (gameState->board[x1][y1]==EMPTY &&
            gameState->board[x2][y2]==color &&
            gameState->board[x3][y3]==color) {

            int lx = row - dx, ly = col - dy;
            int rx = x3 + dx,  ry = y3 + dy;
            if ((in(lx,ly)&&gameState->board[lx][ly]==EMPTY) &&
                (in(rx,ry)&&gameState->board[rx][ry]==EMPTY)) {
                return 1;
            }
        }
    }
    //模式二：X _ X X
    x1 = row - dx;   y1 = col - dy;
    x2 = row + dx;   y2 = col + dy;
    x3 = row + 2*dx; y3 = col + 2*dy;
    if (in(x1,y1)&&in(x2,y2)&&in(x3,y3)) {
        if (gameState->board[x1][y1]==color &&
            gameState->board[x2][y2]==EMPTY &&
            gameState->board[x3][y3]==color) {
            int lx = x1 - dx, ly = col - dy;
            int rx = x3 + dx,  ry = y3 + dy;
            if ((in(lx,ly)&&gameState->board[lx][ly]==EMPTY) &&
                (in(rx,ry)&&gameState->board[rx][ry]==EMPTY)) {
                return 1;
            }
        }
    }
    return 0;
}

// 检查必胜棋型（活四/双冲四/冲四活三/双活三）
ll Checkwin(GameState *gameState, int row, int col) {
    //printf("qwq\n");
    CellState color = gameState->board[row][col];
    if (color == EMPTY) return 0;
    Player targetPlayer = (color == BLACK) ? PLAYER_BLACK : PLAYER_WHITE;
    int live4 = 0, rush4 = 0, live3 = 0;
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
            return 1e14;
        }
        //printf("%d\n",i);

    }
    //printf("qwq");
    for (int d = 0; d < 4; d++) {
       // printf("111qwq\n");
        int line[11];
        for (int i = -5; i <= 5; i++) {
            if (i == 0) {
                line[i + 5] = 1;  // 假设当前位置落子
            } else {
                int r = row + dir[d][0] * i;
                int c = col + dir[d][1] * i;
                line[i + 5] = getCell(gameState, r, c, color);
            }
        }
        // printf("%d:",d);
        // for(int i=0;i<11;i++) printf("%d ",line[i]);
        // printf("\n"); 
        if(line[4]==1&&line[3]==0&&line[2]==1&&line[6]==1&&line[7]==0&&line[8]==1) return 1e14;
        int type = 0;
        int cnt = 0, empty = 0, block = 0;
        for(int i = 1; i <= 5;i++){
            if (line[i] == 1) cnt++;
            else if (line[i] == 0) empty++;
            else block++;
        }
        for (int i = 1; i <= 5; i++) {
            if(i !=1){
                if (line[i-1] == 1) cnt--;
                else if (line[i-1] == 0) empty--;
                else block--;
                if (line[i+4] == 1) cnt++;
                else if (line[i+4] == 0) empty++;
                else block++;
            }
            if (block) continue;
            if(cnt == 4 && empty == 1){
                if(line[i] != 0 && line[i+4] != 0) type = (type < 2) ? 2: type;//空格在中间
                else{
                    int left  = line[i - 1];
                    int right = line[i + 5];
                    if(line[i] == 0) left = 0;
                    if(line[i+4] == 0) right = 0;
                    if (left == 0 && right == 0) type = (type < 3) ? 3: type;
                    else type = type = (type < 2) ? 2: type; 
                }
            }
            else if (cnt == 3 && empty == 2) {
                if(line[i] != 0 && line[i+4] !=0) continue;
                int left  = line[i - 1];
                int right = line[i + 5];
                if(line[i] == 0) left = 0;
                if(line[i+4] == 0) right = 0;
                if (left == 0 && right == 0) type = (type < 1) ? 1: type;
            }
        }
        if(type == 3) live4++;
        if(type == 2) rush4++;
        if(type == 1) live3++;
    }
    //printf("%d %d %d\n",live4,rush4,live3);
    /* 活四 */
    if (live4 >= 1)
        return 1e14;

    /* 双冲四 */
    if (rush4 >= 2)
        return 1e14;

    /* 冲四 + 活三 */
    if (rush4 >= 1 && live3 >= 1)
        return 1e13;

    /* 双活三（最弱必胜） */
    if (live3 >= 2)
        return 1e12;
    return 0;
}

// 检查冲四威胁（返回封堵位置）
int checkFourThreat(GameState *gameState, CellState color, int *blockRow, int *blockCol) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState->board[i][j] != EMPTY) continue;
            gameState->board[i][j] = color;
            if (checkWin(gameState, i, j)) {
                gameState->board[i][j] = EMPTY;
                *blockRow = i;
                *blockCol = j;
                return 2;
            }
            gameState->board[i][j] = EMPTY;
        }
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState->board[i][j] != color) continue;
            for (int d = 0; d < 4; d++) {
                int count = 1;
                int emptyPos[2][2] = {{-1,-1}, {-1,-1}};
                int emptyCnt = 0;
                int dx = dir[d][0], dy = dir[d][1];
                int nx = i + dx, ny = j + dy;
                while (in(nx, ny) && emptyCnt < 2) {
                    if (gameState->board[nx][ny] == color) {
                        count++;
                    } else if (gameState->board[nx][ny] == EMPTY && emptyCnt < 2) {
                        emptyPos[emptyCnt][0] = nx;
                        emptyPos[emptyCnt][1] = ny;
                        emptyCnt++;
                        count++;
                    } else {
                        break;
                    }
                    nx += dx;
                    ny += dy;
                }

                dx = dir[d+4][0], dy = dir[d+4][1];
                nx = i + dx, ny = j + dy;
                while (in(nx, ny) && emptyCnt < 2) {
                    if (gameState->board[nx][ny] == color) {
                        count++;
                    } else if (gameState->board[nx][ny] == EMPTY && emptyCnt < 2) {
                        emptyPos[emptyCnt][0] = nx;
                        emptyPos[emptyCnt][1] = ny;
                        emptyCnt++;
                        count++;
                    } else {
                        break;
                    }
                    nx += dx;
                    ny += dy;
                }

                if (count == 5 && emptyCnt == 1) {
                    *blockRow = emptyPos[0][0];
                    *blockCol = emptyPos[0][1];
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 计算落子的局部评分
long long calculateLocalScore(GameState *gameState, int row, int col, int isAI) {
    long long val = Checkwin(gameState,row,col);
    if(val){ 
        return val;
    } 
    long long localScore = 0;
    int live2Count = 0, sleep2Count = 0, live3Count = 0;
    int sleep3Count = 0, rush4Count = 0, live4Count = 0;
    int color = gameState->board[row][col];
    int AIcolor, playerColor;
    if (isAI) {
        AIcolor = color;
        playerColor = (color == BLACK) ? WHITE : BLACK;
    } else {
        playerColor = color;
        AIcolor = (color == BLACK) ? WHITE : BLACK;
    }
    for (int d = 0; d < 4; d++) {
        int sameCntLeft = 1 , sameCntRight = 0, diffCntLeft = 0, diffCntRight = 0; // 分别比较两侧的步数,当前位置记录到f中
        int leftEmpty = 0, rightEmpty = 0;
        int dx = dir[d][0], dy = dir[d][1];
        int nx = row + dx, ny = col + dy;
        while (in(nx, ny) && gameState->board[nx][ny] == color) {
            sameCntLeft++;
            nx += dx;
            ny += dy;
        }
        if (in(nx, ny) && gameState->board[nx][ny] == EMPTY) leftEmpty = 1;
        nx = row + dx, ny = col + dy;
        while (in(nx, ny) && gameState->board[nx][ny] != color && gameState->board[nx][ny] != EMPTY) {
            diffCntLeft++;
            nx += dx;
            ny += dy;
        }
        dx = dir[d+4][0];
        dy = dir[d+4][1];
        nx = row + dx;
        ny = col + dy;
        while (in(nx, ny) && gameState->board[nx][ny] == color) {
            sameCntRight++;
            nx += dx;
            ny += dy;
        }
        if (in(nx, ny) && gameState->board[nx][ny] == EMPTY) rightEmpty = 1;
        nx = row + dx;
        ny = col + dy;
        while (in(nx, ny) && gameState->board[nx][ny] != color && gameState->board[nx][ny] != EMPTY) {
            diffCntRight++;
            nx += dx;
            ny += dy;
        }
        if(sameCntLeft + sameCntRight == 4 && leftEmpty && rightEmpty) {
            localScore+=chessWeights[0];
        } else if (sameCntLeft + sameCntRight == 4 && (leftEmpty || rightEmpty)) {
            localScore+=chessWeights[1];
        } else if ((sameCntLeft + sameCntRight == 3 && leftEmpty && rightEmpty) || (isBrokenLive3(gameState, row, col, d, color) || isBrokenLive3(gameState, row, col, d+4, color))) {
            localScore+=chessWeights[2];
        } else if (sameCntLeft + sameCntRight == 3 && (leftEmpty || rightEmpty) ) {
            localScore+=chessWeights[3];
        } else if (sameCntLeft + sameCntRight == 2 && leftEmpty && rightEmpty) {
            localScore+=chessWeights[4];
        } else if (sameCntLeft + sameCntRight == 2 && (leftEmpty || rightEmpty)) {
            localScore+=chessWeights[5];
        }
        //堵的行为有以下几种，1.将两侧的活三堵住，2.将两侧的断三堵住，（如果是断四或者更高，那么已经堵了），冲四也不用管，一定会堵，双冲四在必赢中判断过了。
        if((diffCntLeft==3 && leftEmpty) || (diffCntRight==3 && rightEmpty) || (diffCntLeft==1 && diffCntRight==2 && leftEmpty && rightEmpty) || (diffCntRight==1 && diffCntLeft==2 && leftEmpty && rightEmpty)){  
            localScore += chessWeights[2]; 
        } //将眠三堵住了
        else if((diffCntLeft==3 && !leftEmpty) || (diffCntRight==3 && !rightEmpty)){
            localScore += chessWeights[3];
        }
        else if((diffCntLeft==2 && leftEmpty) || (diffCntRight==2 && rightEmpty)) {
            localScore+=chessWeights[4];
        } 
        else if ((diffCntLeft==2 && !leftEmpty) || (diffCntRight==2 && !rightEmpty)) {
            localScore+=chessWeights[5];
        }
        // 现在考虑减少的原来的棋形
        if((sameCntLeft==4 && leftEmpty) || (sameCntRight==3 && rightEmpty) || (sameCntLeft==2 && sameCntRight==2 && leftEmpty && rightEmpty) || (sameCntRight== 1 && sameCntLeft== 3 && leftEmpty && rightEmpty)){  
            localScore -= chessWeights[2]; 
        } //将眠三堵住了
        else if((sameCntLeft==4 && !leftEmpty) || (sameCntRight==3 && !rightEmpty)){
            localScore -= chessWeights[3];
        }
        else if((sameCntLeft==3 && leftEmpty) || (diffCntRight==2 && rightEmpty)) {
            localScore-=chessWeights[4];
        } 
        else if ((sameCntLeft==3 && !leftEmpty) || (sameCntRight==2 && !rightEmpty)) {
            localScore-=chessWeights[5];
        }
    }
    return localScore;
}

// 更新全局局面评分
void updateScore(GameState *gameState, int row, int col, int isAI) {
    if(isAI) gameState->Score += calculateLocalScore(gameState, row, col, 1);
    else gameState->Score -= calculateLocalScore(gameState, row, col, 0);
}