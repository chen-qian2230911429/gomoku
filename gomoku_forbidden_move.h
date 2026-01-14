#ifndef GOMOKU_FORBIDDEN_MOVE_H
#define GOMOKU_FORBIDDEN_MOVE_H

#include "gomoku_base.h"  // 依赖基础定义（GameState/CellState等）

void getLine(GameState *gameState, int row, int col,
             int dr, int dc, CellState color, int line[9]);

// 活三计数（判断是否为活三）

int countLiveThree(int line[9]);

// 检查是否有长连（超过5子，禁手）
int hasOverline(int line[9]);

// 冲四/活四计数（判断是否为冲四）
int countFour(int line[9]);

// 核心禁手判断（检查落子是否合法，含三三/四四/长连禁手）
int isValidMove(GameState *gameState, int row, int col, CellState color);

#endif // GOMOKU_FORBIDDEN_MOVE_H