#include <math.h>
#include <stdio.h>
#include "include/raylib.h"

void printBoard(int board[4][4]);
void generateTile(int board[4][4]);
bool isFullBoard(int board[4][4]);
void gameOver();

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	int boardState[4][4] = {{0, 0, 0, 0}, 
							{0, 0, 0, 0},
							{0, 0, 0, 0},
							{0, 0, 0, 0}};

	generateTile(boardState);
	generateTile(boardState);
	int a = 10;
	int b = 32;
	
	while (!WindowShouldClose()) {
		if (isFullBoard(boardState)) gameOver();
		int input = GetKeyPressed();
		switch (input) {
			case KEY_UP:
				for (int i = 1; i < 4; ++i) {
					for (int j = 0; j < 4; ++j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						for (int k = 0; k < i; ++k) {
							int collidedValue = boardState[i-k-1][j];
							if (collidedValue == 0) {
								boardState[i-k-1][j] = value;
								boardState[i-k][j] = 0;
							} else if (collidedValue == value) {
								printf("collision\n");
								//boardState[i-k-1][j] = value + value;
								//boardState[i-k][j] = 0;
							}
						}
					}
				}
				generateTile(boardState);
				break;
			case KEY_DOWN:
				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						for (int k = 0; k < 3-i; ++k) {
							int collidedValue = boardState[i+k+1][j];
							if (collidedValue == 0) {
								boardState[i+k+1][j] = value;
								boardState[i+k][j] = 0;
							} else if (collidedValue == value) {
								printf("collision\n");
								//boardState[i+k+1][j] = value + value;
								//boardState[i+k][j] = 0;
							}
						}
					}
				}
				break;
			case KEY_LEFT:
				for (int i = 0; i < 4; ++i) {
					for (int j = 1; j < 4; ++j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						for (int k = 0; k < j; ++k) {
							int collidedValue = boardState[i][j-k-1];
							if (collidedValue == 0) {
								boardState[i][j-k-1] = value;
								boardState[i][j-k] = 0;
							} else if (collidedValue == value) {
								printf("collision\n");
								//boardState[i][j-k-1] = value + value;
								//boardState[i][j-k] = 0;
							}
						}
					}
				}
				generateTile(boardState);
				break;
			case KEY_RIGHT:
				for (int i = 0; i < 4; ++i) {
					for (int j = 2; j >= 0; --j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						for (int k = 0; k < 3-j; ++k) {
							int collidedValue = boardState[i][j+k+1];
							if (collidedValue == 0) {
								boardState[i][j+k+1] = value;
								boardState[i][j+k] = 0;
							} else if (collidedValue == value) {
								printf("collision\n");
								//boardState[i][j+k+1] = value + value;
								//boardState[i][j+k] = 0;
							}
						}
					}
				}
				generateTile(boardState);
				break;
		}
		
		int sum = 0;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				sum += boardState[i][j];
			}
		}
		printf("Board sum: %d\n", sum);
		BeginDrawing();
			ClearBackground(RAYWHITE);
			//Draw Board background
			float boardDim = fminf(screenSize.x*0.8f, screenSize.y*0.8f);
			Vector2 boardPos = (Vector2){screenSize.x / 2 - boardDim / 2, screenSize.y / 2 - boardDim / 2};
			DrawRectangleRounded((Rectangle){boardPos.x, boardPos.y, boardDim, boardDim}, 0.05f, 0, DARKGREEN);
			float thick = 10;
			float innerDim = (boardDim - 5 * thick) / 4;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					DrawRectangleRounded(
						(Rectangle){
							boardPos.x + innerDim * j + 10*(j+1), 
							boardPos.y + innerDim * i + 10*(i+1),
							innerDim, 
							innerDim
						},
						0.05f, 0, GREEN
					);
				}
			}
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					int value = boardState[i][j];
					if (value == 0) continue;

					DrawRectangleRounded(
						(Rectangle){
							boardPos.x + innerDim * j + 10*(j+1), 
							boardPos.y + innerDim * i + 10*(i+1),
							innerDim, 
							innerDim
						},
						0.05f, 0, YELLOW
					);
					DrawText(TextFormat("%d", value), 
						boardPos.x + innerDim * j + 10*(j+1) + 40, 
						boardPos.y + innerDim * i + 10*(i+1) + 20,
						100, BLACK);
				}
			}
			//Draw board
		EndDrawing();
	}
	return 0;
}

void generateTile(int board[4][4]) {
	int possibleNums[] = {2,4};
	int newIndex;
	do {
		newIndex = GetRandomValue(0, 15);
	} while (board[newIndex % 4][newIndex / 4] != 0);
	int newNum = possibleNums[GetRandomValue(0,1)];
	board[newIndex % 4][newIndex / 4] = newNum;
}

void printBoard(int board[4][4]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", board[i][j]);
		}
		printf("\n");
	}
}

bool isFullBoard(int board[4][4]) {
	int fullSpaces = 0;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (board[i][j] != 0) fullSpaces++;
		}
	}
	return (fullSpaces == 16);
}

void gameOver() {
	CloseWindow();
}
