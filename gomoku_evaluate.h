#ifndef GOMOKU_EVALUATE_H
#define GOMOKU_EVALUATE_H

#include "gomoku_base.h"  // 依赖基础定义（GameState/CellState/BOARD_SIZE等）

// 声明函数接口
// 判断断活三（假活三）
int isBrokenLive3(GameState *gameState, int row, int col, int d, CellState color);

// 检查必胜棋型（活四/双冲四/冲四活三/双活三）
ll Checkwin(GameState *gameState, int row, int col);

// 检查冲四威胁（返回需要封堵的位置）
int checkFourThreat(GameState *gameState, CellState color, int *blockRow, int *blockCol);

// 计算落子的局部评分
long long calculateLocalScore(GameState *gameState, int row, int col, int isAI);

// 更新全局局面评分
void updateScore(GameState *gameState, int row, int col, int isAI);


#endif // GOMOKU_EVALUATE_H