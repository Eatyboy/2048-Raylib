#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "include/raylib.h"
#include "include/raymath.h"

typedef struct Anim {
	int dx;
	int dy;
	float t;
} Anim;

typedef struct Tile {
	int num;
	Anim *anim;
} Tile;

void printBoard(int board[4][4]);
void printTiles(Tile tiles[4][4]);
void generateTile(int board[4][4], Tile tiles[4][4]);
bool isFullBoard(int board[4][4]);
void gameOver();

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	const float animDt = 1.0f / 10.0f;

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	int boardState[4][4] = {{0, 0, 0, 0}, 
							{0, 0, 0, 0},
							{0, 0, 0, 0},
							{0, 0, 0, 0}};
	Tile tiles[4][4];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			tiles[i][j] = (Tile){
				0,
				NULL
			};
		}
	}
	int activeAnims = 0;
	bool shouldSpawnTile = false;

	generateTile(boardState, tiles);
	generateTile(boardState, tiles);

	while (!WindowShouldClose()) {
		if (isFullBoard(boardState)) gameOver();

		int input = GetKeyPressed();
		switch (input) {
			case KEY_UP:
				for (int i = 1; i < 4; ++i) {
					for (int j = 0; j < 4; ++j) {
						int value = tiles[i][j].num;
						if (value == 0) continue;
						int delta = 0;
						for (int k = i-1; k >= 0; k--) {
							int collidedValue = tiles[k][j].num;
							if (collidedValue == 0) {
								delta++;
							} else {
								break;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = -delta;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
						}
					}
				}
				shouldSpawnTile = true;
				break;
			case KEY_DOWN:
				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						int delta = 0;
						for (int k = i+1; k <= 3; ++k) {
							int collidedValue = tiles[k][j].num;
							if (collidedValue == 0) {
								delta++;
							} else {
								break;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = delta;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
						}
					}
				}
				shouldSpawnTile = true;
				break;
			case KEY_LEFT:
				for (int i = 0; i < 4; ++i) {
					for (int j = 1; j < 4; ++j) {
						int value = tiles[i][j].num;
						if (value == 0) continue;
						int delta = 0;
						for (int k = j-1; k >= 0; k--) {
							int collidedValue = tiles[i][k].num;
							if (collidedValue == 0) {
								delta++;
							} else {
								break;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = -delta;
							newAnim->dy = 0;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
						}
					}
				}
				shouldSpawnTile = true;
				break;
			case KEY_RIGHT:
				for (int i = 0; i < 4; ++i) {
					for (int j = 2; j >= 0; --j) {
						int value = boardState[i][j];
						if (value == 0) continue;
						int delta = 0;
						for (int k = j+1; k <= 3; ++k) {
							int collidedValue = tiles[i][k].num;
							if (collidedValue == 0) {
								delta++;
							} else {
								break;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = delta;
							newAnim->dy = 0;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
						}
					}
				}
				shouldSpawnTile = true;
				break;
		}

		if (activeAnims > 0) {
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					Anim *anim = tiles[i][j].anim;
					if (anim == NULL) continue;
					if (!FloatEquals(anim->t, 0.0f)) {
						anim->t = fmaxf(anim->t - animDt, 0.0f);
					} else {
						Tile temp = tiles[i][j];
						tiles[i][j] = tiles[i + anim->dy][j + anim->dx];
						tiles[i + anim->dy][j + anim->dx] = temp;
						tiles[i + anim->dy][j + anim->dx].anim = NULL;
						free(anim);
						activeAnims--;
					}
				}
			}
		}

		if (shouldSpawnTile && activeAnims == 0) {
			generateTile(boardState, tiles);
			shouldSpawnTile = false;
		}

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

			// Draw tiles
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					Tile tile = tiles[i][j];

					if (tile.num == 0) continue;

					float tileX = (tile.anim != NULL) 
						? Lerp(j, j + tile.anim->dx, 1 - tile.anim->t)
						: j;
					float tileY = (tile.anim != NULL)
						? Lerp(i, i + tile.anim->dy, 1 - tile.anim->t)
						: i;

					DrawRectangleRounded(
						(Rectangle){
							boardPos.x + thick + tileX * (innerDim + thick),
							boardPos.y + thick + tileY * (innerDim + thick),
							innerDim, 
							innerDim
						},
						0.05f, 0, YELLOW
					);
					DrawText(TextFormat("%d", tile.num), 
						boardPos.x + 40 + tileX * (innerDim + thick),
						boardPos.y + 20 + tileY * (innerDim + thick),
						100, BLACK);
				}
			}
			//Draw board
		EndDrawing();
	}
	return 0;
}

void generateTile(int board[4][4], Tile tiles[4][4]) {
	int possibleNums[] = {2,4};
	int newIndex;
	do {
		newIndex = GetRandomValue(0, 15);
	} while (board[newIndex % 4][newIndex / 4] != 0);
	int newNum = possibleNums[GetRandomValue(0,1)];
	board[newIndex % 4][newIndex / 4] = newNum;
	tiles[newIndex % 4][newIndex / 4] = (Tile){
		newNum,
		NULL
	};
}

void printBoard(int board[4][4]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", board[i][j]);
		}
		printf("\n");
	}
}

void printTiles(Tile tiles[4][4]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%d ", tiles[i][j].num);
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
	printf("you lose");
	CloseWindow();
}
