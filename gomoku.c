#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> 
#endif
#include "gomoku_base.h" //基础定义
#include "gomoku_forbidden_move.h" //禁手
#include "gomoku_evaluate.h" //估价相关函数
#define BOARD_SIZE 15
#define SEARCH_DEPTH 10 //搜索层数8层
#define ll long long
clock_t searchStart;
double timeLimit = 14.7;   
int timeUp = 0;

// 置换表类型枚举
typedef enum {
    TT_EXACT,   
    TT_ALPHA,   
    TT_BETA     
} TTType;

// 置换表条目结构体
typedef struct {
    unsigned long long hash;  
    int depth;                
    long long value;          
    TTType type;              
} TTEntry;

// 常量定义
#define TT_SIZE 5000011  


TTEntry TT[TT_SIZE] ={0};                          
unsigned long long zobristTable[BOARD_SIZE][BOARD_SIZE][3];  
unsigned long long currentHash = 0;    

void initZobrist() {
    srand(time(NULL));  // 设置随机数种子
    for(int i=0; i<BOARD_SIZE; i++) {
        for(int j=0; j<BOARD_SIZE; j++) {
            for(int k=0; k<3; k++) {
                // 生成64位随机数（通过多个32位随机数拼接）
                unsigned long long r1 = (unsigned long long)rand();
                unsigned long long r2 = (unsigned long long)rand();
                unsigned long long r3 = (unsigned long long)rand();
                // 拼接为64位：r1(16位)<<48 + r2(16位)<<32 + r3(16位)<<16 + r1(16位)
                zobristTable[i][j][k] = (r1 << 48) | (r2 << 32) | (r3 << 16) | r1;
            }
        }
    }
    currentHash = 0;  // 空棋盘哈希值重置为0
}


void updateHash(int row, int col, int oldColor, int newColor) {

    int oldIdx = (oldColor == EMPTY) ? 0 : (oldColor == BLACK ? 1 : 2);
    int newIdx = (newColor == EMPTY) ? 0 : (newColor == BLACK ? 1 : 2);
    currentHash ^= zobristTable[row][col][oldIdx];
    currentHash ^= zobristTable[row][col][newIdx];
}

/* root 层最优解 */
int currentRootDepth = 0;
int rootBestRow = -1;
int rootBestCol = -1;

static inline int isTimeUp() { //时间检查
    return ((double)(clock() - searchStart) / CLOCKS_PER_SEC) >= timeLimit;
}

// 检查是否获胜

void initializeGame(GameState *gameState) {
    gameState->currentPlayer = PLAYER_BLACK;
    gameState->lastMove.row = -1;
    gameState->lastMove.col = -1;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            gameState->board[i][j] = EMPTY;
        }
    }
}

// 打印棋盘（完全按你提供的代码，无修改）
void printBoard(GameState *gameState) {
    if (gameState->lastMove.row != -1 && gameState->lastMove.col != -1) {
        printf("上一步落子：%c%d\n", (char)gameState->lastMove.col + 'A', BOARD_SIZE - gameState->lastMove.row);
    } else {
        printf("棋盘尚未落子！\n");
    }
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", BOARD_SIZE - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState->board[i][j] == BLACK) {
                if (j == BOARD_SIZE - 1) {
                    if (i == gameState->lastMove.row && j == gameState->lastMove.col)
                        printf("▲\n");
                    else
                        printf("●\n");
                } else {
                    if (i == gameState->lastMove.row && j == gameState->lastMove.col)
                        printf("▲─");
                    else
                        printf("●─");
                }
            } else if (gameState->board[i][j] == WHITE) {
                if (j == BOARD_SIZE - 1) {
                    if (i == gameState->lastMove.row && j == gameState->lastMove.col)
                        printf("△\n");
                    else
                        printf("◎\n");
                } else {
                    if (i == gameState->lastMove.row && j == gameState->lastMove.col)
                        printf("△─");
                    else
                        printf("◎─");
                }
            } else if (i == 0) {
                if (j == 0) { printf("┌─"); }
                else if (j == BOARD_SIZE - 1) { printf("┐\n"); }
                else { printf("┬─"); }
            } else if (i == BOARD_SIZE - 1) {
                if (j == 0) { printf("└─"); }
                else if (j == BOARD_SIZE - 1) { printf("┘\n"); }
                else { printf("┴─"); }
            } else {
                if (j == 0) { printf("├─"); }
                else if (j == BOARD_SIZE - 1) { printf("┤\n"); }
                else { printf("┼─"); }
            }
        }
    }

    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++)
        printf(" %c", 'A' + i);
    printf("\n");
}

// 标记搜索区域
void markSearchAreaLocal(GameState *gameState, int area[15][15]) {
    memset(area, 0, sizeof(int)*15*15);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState->board[i][j] != EMPTY) {
                for(int d=0;d<8;d++){
                    for(int k=1;k<=2;k++){
                        int nx = i + dir[d][0]*k;
                        int ny = j + dir[d][1]*k;
                        if (in(nx, ny)) {
                            if(area[nx][ny]==0&&gameState->board[nx][ny]==EMPTY) area[nx][ny]=1; 
                        }
                    }
                }
            }
        }
    }
}



int forbidRow=-1,forbidCol = -1;
long long minimax(GameState *gameState, int depth,
                  long long alpha, long long beta, int isMax){
    if (isTimeUp()) {
        timeUp = 1;
        return gameState->Score;
    }
    unsigned long long hash = currentHash;
    int idx = hash % TT_SIZE;
    TTEntry *entry = &TT[idx];

    long long alphaOrig = alpha;

    /* ---------- TT 查询 ---------- */
    if (entry->hash == hash && entry->depth >= depth) {
        if (entry->type == TT_EXACT)
            return entry->value;   
        if (entry->type == TT_ALPHA && entry->value <= alpha)
            return entry->value;
        if (entry->type == TT_BETA && entry->value >= beta)
            return entry->value;
    }

    CellState AIcolor, playercolor;
    if (gameState->currentPlayer == PLAYER_BLACK) {
        AIcolor = BLACK;
        playercolor = WHITE;
    } else {
        AIcolor = WHITE;
        playercolor = BLACK;
    }

    if (depth == 0)
        return gameState->Score;

    int localArea[15][15], vis[15][15], In[15][15];
    memset(vis, 0, sizeof(vis));
    memset(In, 0, sizeof(In));
    markSearchAreaLocal(gameState, localArea);

    CandidatePos moves[205];
    int moveCnt = 0;

    /* ---------- 生成候选 ---------- */
    
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 15; j++) {
            if (localArea[i][j] &&
                gameState->board[i][j] == EMPTY &&
                isValidMove(gameState, i, j,
                            isMax ? AIcolor : playercolor)) {

                gameState->board[i][j] = isMax ? AIcolor : playercolor;
                long long s = calculateLocalScore(gameState, i, j, isMax);
                gameState->board[i][j] = EMPTY;
                
                moves[moveCnt++] = (CandidatePos){i, j, s};
            }
        }
    }
    sortCandidates(moves, moveCnt);
    CandidatePos cand[20];
    int cnt = 0;

    /* ================= MAX ================= */
    if (isMax) {
        long long maxEval = -1e18;
        int bestrow = -1, bestcol = -1;
        for (int k = -1; k < min(20, moveCnt); k++) {
            
            int i = moves[k].row, j = moves[k].col;
            if (gameState->board[i][j] != EMPTY) continue;
            long long nowscore = gameState->Score;
            gameState->board[i][j] = AIcolor;
            
            updateHash(i, j, EMPTY, AIcolor);
            vis[i][j] = 1;
            if (Checkwin(gameState, i, j) == 1e14) {
                gameState->board[i][j] = EMPTY;
                updateHash(i, j, AIcolor, EMPTY);
                gameState->Score = nowscore;
                forbidRow = i;
                forbidCol = j;
                if (depth == currentRootDepth) {
                    rootBestRow = i;
                    rootBestCol = j;
                }
                return 1e14;
            }
            long long eval;
           if(k == 0){
                eval = minimax(gameState, depth - 1,
                                     alpha, beta, 0);
            }
            else {
                eval = minimax(gameState,depth-1, alpha, alpha+1, 0); // 零窗口
                if (eval > alpha && eval < beta) {
                    eval = minimax(gameState,depth-1, alpha, beta, 0); // re-search
                }
            }

            if (eval < -1e11 && !In[forbidRow][forbidCol] && cnt < 10) {
                cand[++cnt].row = forbidRow;
                cand[cnt].col = forbidCol;
                In[forbidRow][forbidCol] = 1;
            }

            gameState->board[i][j] = EMPTY;
            updateHash(i, j, AIcolor, EMPTY);
            gameState->Score = nowscore;
            if (eval > maxEval) {
                maxEval = eval;
                bestrow = i;
                bestcol = j;
                if (depth == currentRootDepth) {
                    rootBestRow = i;
                    rootBestCol = j;
                }
            }

            alpha = max(alpha, eval);

            if (alpha >= beta)
                break;
        }

        /* cand 扫描 */
        for (int k = 1; k <= cnt; k++) {
            int i = cand[k].row, j = cand[k].col;
            if (vis[i][j] || gameState->board[i][j] != EMPTY) continue;
            if (!isValidMove(gameState, i, j, AIcolor)) continue;
            long long nowscore = gameState->Score;
            gameState->board[i][j] = AIcolor;
            updateScore(gameState, i, j, 1);
            updateHash(i, j, EMPTY, AIcolor);
            long long eval;
            
            eval = minimax(gameState,depth-1, alpha, alpha+1, 0); // 零窗口
            if (eval > alpha && eval < beta) {
                eval = minimax(gameState,depth-1, alpha, beta, 0); // re-search
            }
            gameState->board[i][j] = EMPTY;
            updateHash(i, j, AIcolor, EMPTY);
            gameState->Score = nowscore;

            if (eval > maxEval) {
                maxEval = eval;
                bestrow = i;
                bestcol = j;
                if (depth == currentRootDepth) {
                    rootBestRow = i;
                    rootBestCol = j;
                }
            }

            alpha = max(alpha, eval);
            if (alpha >= beta)
                break;
        }

        /* ---------- TT 写入 ---------- */
        TTType type;
        if (maxEval <= alphaOrig)
            type = TT_ALPHA;
        else if (maxEval >= beta)
            type = TT_BETA;
        else
            type = TT_EXACT;

        if (entry->hash != hash || entry->depth <= depth) {
            entry->hash  = hash;
            entry->depth = depth;
            entry->value = maxEval;
            entry->type  = type;
        }

        if (maxEval > 1e11) {
            forbidRow = bestrow;
            forbidCol = bestcol;
        }

        return maxEval;
    }

    /* ================= MIN ================= */
    else {
        long long minEval = 1e18;
        int bestrow = -1, bestcol = -1;

        for (int k = 0; k < min(20, moveCnt); k++) {
            int i = moves[k].row, j = moves[k].col;
            if (gameState->board[i][j] != EMPTY) continue;
            long long nowscore = gameState->Score;
            gameState->board[i][j] = playercolor;
            gameState->Score -= moves[k].score;
            vis[i][j] = 1;
            if (Checkwin(gameState, i, j) == 1e14) {
                gameState->board[i][j] = EMPTY;
                updateHash(i, j, playercolor, EMPTY);
                gameState->Score = nowscore;
                forbidRow = i;
                forbidCol = j;
                return -1e14;
            }

            long long eval;
            if(k == 0){
                eval = minimax(gameState, depth - 1,
                                     alpha, beta, 1);    
            }
            else{
                eval = minimax(gameState,depth-1, beta-1, beta, 1);
                if (eval < beta && eval > alpha) {
                    eval = minimax(gameState,depth-1, alpha, beta, 1);
                }
            }
            if (eval > 1e11 && !In[forbidRow][forbidCol] && cnt < 10) {
                cand[++cnt].row = forbidRow;
                cand[cnt].col = forbidCol;
                In[forbidRow][forbidCol] = 1;
            }
            gameState->board[i][j] = EMPTY;
            updateHash(i, j, playercolor, EMPTY);
            gameState->Score = nowscore;
            if (eval < minEval) {
                minEval = eval;
                bestrow = i;
                bestcol = j;
            }

            beta = min(beta, eval);
            if (beta <= alpha)
                break;
        }
        for (int k = 1; k <= cnt; k++) {
            int i = cand[k].row, j = cand[k].col;
            if (vis[i][j] || gameState->board[i][j] != EMPTY) continue;
            if (!isValidMove(gameState, i, j, playercolor)) continue;
            long long nowscore = gameState->Score;
            gameState->board[i][j] = playercolor;
            updateScore(gameState, i, j, 0);
            updateHash(i, j, EMPTY, playercolor);
            long long eval =  eval = minimax(gameState,depth-1, beta-1, beta, 1);
            if (eval < beta && eval > alpha) {
                eval = minimax(gameState,depth-1, alpha, beta, 1);
            }
            gameState->board[i][j] = EMPTY;
            updateHash(i, j, playercolor, EMPTY);
            gameState->Score = nowscore;
            if (eval < minEval) {
                minEval = eval;
                bestrow = i;
                bestcol = j;
            }
            beta = min(beta, eval);
            if (beta <= alpha)
                break;
        }

        /* ---------- TT 写入 ---------- */
        TTType type;
        if (minEval >= beta)
            type = TT_BETA;
        else if (minEval <= alphaOrig)
            type = TT_ALPHA;
        else
            type = TT_EXACT;

        if (entry->hash != hash || entry->depth <= depth) {
            entry->hash  = hash;
            entry->depth = depth;
            entry->value = minEval;
            entry->type  = type;
        }

        if (minEval < -1e11) {
            forbidRow = bestrow;
            forbidCol = bestcol;
        }

        return minEval;
    }
}
// AI落子逻辑
void aiMove(GameState *gameState, int *row, int *col) {
    searchStart = clock();
    timeUp = 0;
    CellState AIcolor, playercolor;
    if (gameState->currentPlayer == PLAYER_BLACK) {
        AIcolor = BLACK;
        playercolor = WHITE;
    } else {
        AIcolor = WHITE;
        playercolor = BLACK;
    }
    int blockRow = -1, blockCol = -1;
    int threatType;

    // 优先自己成五
    threatType = checkFourThreat(gameState, AIcolor, &blockRow, &blockCol);
    if (threatType == 2) {
        if (isValidMove(gameState, blockRow, blockCol, AIcolor)) {
            *row = blockRow;
            *col = blockCol;
            return;
        }
    }

    // 其次堵对手成五
    threatType = checkFourThreat(gameState, playercolor, &blockRow, &blockCol);
    if (threatType == 2) {
        if (isValidMove(gameState, blockRow, blockCol, AIcolor)) {
            *row = blockRow;
            *col = blockCol;
            return;
        }
    }

    // 优先自己活四
    if (checkFourThreat(gameState, AIcolor, &blockRow, &blockCol) == 1) {
        if (isValidMove(gameState, blockRow, blockCol, AIcolor)) {
            *row = blockRow;
            *col = blockCol;
            return;
        }
    }

    // 其次堵对手活四
    if (checkFourThreat(gameState, playercolor, &blockRow, &blockCol) == 1) {
        if (isValidMove(gameState, blockRow, blockCol, AIcolor)) {
            *row = blockRow;
            *col = blockCol;
            return;
        }
    }
    long long bestValue = -1e18;
    int bestRow = -1, bestCol = -1;
    rootBestCol = -1;
    rootBestRow = -1;
    long long nowScore = gameState->Score;
    for (int depth = 2; depth <= SEARCH_DEPTH; depth+=2) {
        currentRootDepth = depth;
        long long val = minimax(gameState, depth, -1e18, 1e18, 1);
        if (timeUp) break;
        bestValue = val;
        bestRow = rootBestRow;
        bestCol = rootBestCol;
        printf("[ID] depth=%d best=%c%d val=%lld\n",
           depth,
           'A' + bestCol,
           BOARD_SIZE - bestRow,
           val);
    }
    gameState->Score = nowScore;
    if (bestRow != -1) {
        *row = bestRow;
        *col = bestCol;
        return;
    }
    
}

// 主函数
int main(int argc, char *argv[]) {
    #ifdef _WIN32
        system("chcp 65001 > nul"); // 切换为UTF-8编码
    #endif
    printf("Welcome to Gomoku!\n");
    // 初始化游戏状态
    GameState gameState;
    initializeGame(&gameState);
    GameMode gameMode;
    char input[100];
    initZobrist();
    // 选择游戏模式
    int mode = 1;
    printf("PVP or PVE? (0: PVP, 1: PVE): ");
    scanf("%d", &mode);
    CellState AIcolor = BLACK, playercolor = BLACK;
    if (mode == 0) {
        gameMode = MODE_PVP;
        printf("You selected Player vs Player mode.\n");
    } else {
        gameMode = MODE_PVE;
        printf("You selected Player vs AI mode.\n");
        printf("Who goes first? (1: You, 2: AI): ");
        int first = 1;
        scanf("%d", &first);

        if (first == 1) {
            gameState.currentPlayer = PLAYER_BLACK; // 玩家黑棋先手
            AIcolor = WHITE;
            playercolor = BLACK;
        } else {
            gameState.currentPlayer = PLAYER_WHITE; // AI黑棋先手
            gameState.board[7][7] = BLACK; // AI落子中心
            gameState.lastMove.row = 7;
            gameState.lastMove.col = 7;
            updateHash(7, 7, EMPTY, BLACK);
            updateScore(&gameState, 7, 7, 1);
            AIcolor = BLACK;
            playercolor = WHITE;
        }
    }
    //printf("AI选择落子位置: %c%d\n", (char)(col + 'A'), BOARD_SIZE - row);
    printBoard(&gameState);
    while (1) {
        // 玩家落子阶段
        printf("请输入您的落子位置 (例如 H8): ");
        scanf("%s", input);
        int col = input[0] - 'A';
        int row = atoi(&input[1]) - 1;
        row = BOARD_SIZE - row - 1;

        // 验证输入合法性
        while (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE || gameState.board[row][col] != EMPTY ||!isValidMove(&gameState, row, col, playercolor)) {
            if(!isValidMove(&gameState, row, col, (gameState.currentPlayer == PLAYER_BLACK) ? BLACK : WHITE) ) printf("此手是禁手，请重新输入。");
            else if(col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) printf("超出棋盘，请重下\n");
            else if(gameState.board[row][col] != EMPTY) printf("输入位置无效，请重新输入。\n");
            scanf("%s", input);
            col = input[0] - 'A';
            row = atoi(&input[1]) - 1;
            row = BOARD_SIZE - row - 1;
        }

        // 执行落子
        gameState.board[row][col] = (gameState.currentPlayer == PLAYER_BLACK) ? BLACK : WHITE;
        gameState.lastMove.row = row;
        gameState.lastMove.col = col;
        updateHash(row, col, EMPTY, gameState.board[row][col]);
        updateScore(&gameState, row, col, 0);
        printBoard(&gameState);
        int winResult = checkWin(&gameState, row, col);
        if (winResult != 0) {
            printf("%s获胜！\n", (winResult == 1) ? "黑方" : "白方");
            break;
        }
        gameState.currentPlayer = (gameState.currentPlayer == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
        if (gameMode == MODE_PVE) {
            clock_t start_time = clock(); // 记录开始时间
            printf("AI 正在思考落子...\n");
            
            aiMove(&gameState, &row, &col); // 原有AI落子逻辑
            
            clock_t end_time = clock(); // 记录结束时间
            double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; // 计算耗时（秒）
            gameState.board[row][col] = AIcolor;
            updateHash(row, col, EMPTY, gameState.board[row][col]);
            updateScore(&gameState, row, col, 1);
            gameState.lastMove.row = row;
            gameState.lastMove.col = col;
            printf("AI 落子耗时：%.3f 秒，落子位置：%c%d\n", 
                   elapsed_time, 
                   (char)gameState.lastMove.col + 'A', 
                   BOARD_SIZE - gameState.lastMove.row); // 打印耗时和AI落子位置
            printBoard(&gameState);
            winResult = checkWin(&gameState, row, col);
            if (winResult != 0) {
                printf("%s获胜！\n", (winResult == 1) ? "黑方" : "白方");
                break;
            }
            gameState.currentPlayer = (gameState.currentPlayer == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
        }
    }
    return 0;
}
/*
gcc -O3 gomoku_base.c gomoku.c gomoku_forbidden_move.c gomoku_evaluate.c -o gomoku.exe -std=c99 -lmsvcrt
*/

