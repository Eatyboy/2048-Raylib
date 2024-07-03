#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug true

#define BWIDTH 4
#define BHEIGHT 4

#define ANIMDT (0.05f)

typedef struct Anim {
	int dx;
	int dy;
	float t;
} Anim;

typedef struct Tile {
	int num;
	Anim *anim;
} Tile;

Color numColors[] = {
	(Color){220, 150, 150, 255},
	(Color){220, 125, 125, 255},
	(Color){220, 100, 100, 255},
	(Color){235, 75, 75, 255}
};

void gameOver();
void restartGame(Tile board[BHEIGHT][BWIDTH], 
				int newState[BHEIGHT][BWIDTH],
				int *animCount,
				bool *spawningTiles);
void printBoard(Tile tiles[BHEIGHT][BWIDTH]);
void printTiles(Tile tiles[BHEIGHT][BWIDTH]);
int generateTile(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount);
bool isFullBoard(Tile board[BHEIGHT][BWIDTH]);
void endAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
void updateAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
bool runTests(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles);

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	#if debug
	int boardCount = 0;
	#endif

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	Tile tiles[BHEIGHT][BWIDTH];
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			tiles[i][j] = (Tile){
				0,
				NULL
			};
		}
	}
	int newState[4][4];
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			newState[i][j] = 0;
		}
	}
	int activeAnims = 0;
	bool shouldSpawnTile = false;

	#if debug
	boardCount +=
	#endif
	generateTile(tiles, newState, &activeAnims);
	#if debug
	boardCount +=
	#endif
	generateTile(tiles, newState, &activeAnims);


	while (!WindowShouldClose()) {
		if (isFullBoard(tiles)) gameOver();

		int input = GetKeyPressed();
		#if debug
		if (input == KEY_SPACE) {
			printBoard(tiles);
		}
		if (input == KEY_B) {
			printTiles(tiles);
		}
		if (input == KEY_R) {
			restartGame(tiles, newState, &activeAnims, &shouldSpawnTile);
			printf("Restarted game\n");
		}
		if (input == KEY_I) {
			printf("Active anims: %d, shouldSpawnTile: %s\n", activeAnims, shouldSpawnTile ? "true" : "false");
			if (activeAnims > 0) {
				for (int i = 0; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						Anim *a = tiles[i][j].anim;
						if (a == NULL) printf("Anim at (%d, %d) is null\n", j, i);
						else printf("Anim at (%d, %d) has dx: %d, dy: %d, t: %f\n", j, i, a->dx, a->dy, a->t);
					}
				}
			}
		}
		int prevBoardCount = boardCount;
		#endif
		switch (input) {
			case KEY_UP:
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;
				for (int i = 1; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;

						for (int k = i-1; k >= 0; k--) {
							int collidedValue = tiles[k][j].num;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							} 
							if (nextNum == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							newState[i][j] = 0;
							newState[i - delta][j] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = -delta;
							newAnim->t = 1;

							tiles[i][j].anim = newAnim;
							activeAnims++;

							shouldSpawnTile = true;
						}
					}
				}
				break;
			case KEY_DOWN:
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;
				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;

						for (int k = i+1; k <= 3; ++k) {
							int collidedValue = tiles[k][j].num;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							} 
							if (nextNum == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							newState[i][j] = 0;
							newState[i + delta][j] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = delta;
							newAnim->t = 1;

							tiles[i][j].anim = newAnim;
							activeAnims++;

							shouldSpawnTile = true;
						}
					}
				}
				break;
			case KEY_LEFT:
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;
				for (int j = 1; j < 4; ++j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;

						for (int k = j-1; k >= 0; k--) {
							int collidedValue = tiles[i][k].num;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0 ) {
								nextNum = collidedValue;
							} 
							if (nextNum == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							newState[i][j] = 0;
							newState[i][j - delta] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = -delta;
							newAnim->dy = 0;
							newAnim->t = 1;

							tiles[i][j].anim = newAnim;
							activeAnims++;

							shouldSpawnTile = true;
						}
					}
				}
				break;
			case KEY_RIGHT:
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;
				for (int j = 2; j >= 0; --j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;

						for (int k = j+1; k <= 3; ++k) {
							int collidedValue = tiles[i][k].num;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							}
							if (nextNum == collidedValue) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							newState[i][j] = 0;
							newState[i][j + delta] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = delta;
							newAnim->dy = 0;
							newAnim->t = 1;

							tiles[i][j].anim = newAnim;
							activeAnims++;

							shouldSpawnTile = true;
						}
					}
				}
				break;
		}

		updateAnims(tiles, &activeAnims);

		if (activeAnims == 0) {
			for (int i = 0; i < BHEIGHT; ++i) {
				for (int j = 0; j < BWIDTH; ++j) {
					tiles[i][j].num = newState[i][j];
				}
			}

			if (shouldSpawnTile) {
				#if debug
				boardCount +=
				#endif
				generateTile(tiles, newState, &activeAnims);
				shouldSpawnTile = false;
			}
		}

		#if debug
		if (prevBoardCount > boardCount) {
			printf("ERROR: board decreased in count\n");
		}
		#endif

		BeginDrawing();
			ClearBackground(RAYWHITE);
			//Draw Board background
			float boardDim = fminf(screenSize.x*0.8f, screenSize.y*0.8f);
			Vector2 boardPos = (Vector2){screenSize.x / 2 - boardDim / 2, screenSize.y / 2 - boardDim / 2};
			DrawRectangleRounded((Rectangle){boardPos.x, boardPos.y, boardDim, boardDim}, 0.05f, 0, DARKGREEN);
			float thick = 10;
			float innerDim = (boardDim - 5 * thick) / 4;
			for (int i = 0; i < BHEIGHT; ++i) {
				for (int j = 0; j < BWIDTH; ++j) {
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
			for (int i = 0; i < BHEIGHT; ++i) {
				for (int j = 0; j < BWIDTH; ++j) {
					Tile tile = tiles[i][j];

					if (tile.num == 0) continue;

					float tileX = (tile.anim != NULL) 
						? Lerp(j, j + tile.anim->dx, 1 - tile.anim->t)
						: j;
					float tileY = (tile.anim != NULL)
						? Lerp(i, i + tile.anim->dy, 1 - tile.anim->t)
						: i;
					float size = 1.0f;
					if (tile.anim != NULL) {
						if (tile.anim->dx == 0 && tile.anim->dy == 0) {
							size -= tile.anim->t;
						}
					}

					DrawRectangleRounded(
						(Rectangle){
							boardPos.x + thick
								+ tileX * (innerDim + thick)
								+ (1.0f - size) * 0.5f * innerDim,
							boardPos.y + thick
								+ tileY * (innerDim + thick)
								+ (1.0f - size) * 0.5f * innerDim,
							innerDim * size, 
							innerDim * size
						},
						0.05f, 0, numColors[(int)log2(tile.num) - 1]
					);
					if (FloatEquals(size, 1.0f)) {
						DrawText(TextFormat("%d", tile.num), 
							boardPos.x + 50 + tileX * (innerDim + thick),
							boardPos.y + 30 + tileY * (innerDim + thick),
							80, BLACK);
					}
				}
			}
			//Draw board
		EndDrawing();
	}
	return 0;
}

int generateTile(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount) {
	int possibleNums[] = {2,4};
	int newIndex;
	do {
		newIndex = GetRandomValue(0, 15);
	} while (tiles[newIndex % BHEIGHT][newIndex / BWIDTH].num != 0);

	int newNum = possibleNums[GetRandomValue(0,1)];
	Anim *newAnim = malloc(sizeof(Anim));
	newAnim->dx = 0;
	newAnim->dy = 0;
	newAnim->t = 1;
	tiles[newIndex % BHEIGHT][newIndex / BWIDTH].anim = newAnim;
	tiles[newIndex % BHEIGHT][newIndex / BWIDTH].num = newNum;
	newState[newIndex % BHEIGHT][newIndex / BWIDTH] = newNum;
	(*animCount)++;

	return newNum;
}

void printBoard(Tile tiles[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			printf("%d ", tiles[i][j].num);
		}
		printf("\n");
	}
	printf("---------\n");
}

void printTiles(Tile tiles[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			printf("num: %d ", tiles[i][j].num);
			if (tiles[i][j].anim != NULL) {
				Anim *a = tiles[i][j].anim;
				printf("dx: %d, dy: %d, t: %f ", a->dx, a->dy, a->t);
			}
		}
		printf("\n");
	}
	printf("-----------------\n");
}

bool isFullBoard(Tile board[BHEIGHT][BWIDTH]) {
	int fullSpaces = 0;
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			if (board[i][j].num != 0) fullSpaces++;
		}
	}
	return (fullSpaces == 16);
}

void gameOver() {
	printf("you lose");
	CloseWindow();
}

void endAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount) {
	printf("Active anims before: %d\n", *animCount);
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			if (tiles[i][j].anim == NULL) continue;
			tiles[i][j].anim->t = 0.0f;
			Anim *a = tiles[i][j].anim;
			printf("ended anim at (%d, %d) going to (%d, %d)\n", j, i, j + a->dx, i + a-> dy);
		}
	}

	updateAnims(tiles, animCount);
	printf("Active anims after: %d\n", *animCount);
}

void restartGame(Tile board[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			board[i][j].num = 0;
			board[i][j].anim = NULL;
			newState[i][j] = 0;
		}
	}

	*animCount = 0;
	*spawningTiles = false;

	generateTile(board, newState, animCount);
	generateTile(board, newState, animCount);
}

void updateAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount) {
	if (*animCount <= 0) return;

	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			Anim *anim = tiles[i][j].anim;
			if (anim == NULL) continue;

			anim->t -= ANIMDT;
			if (anim->t > 0.0f) continue;

			tiles[i][j].anim = NULL;
			free(anim);
			(*animCount)--;
		}
	}
}

bool runTests(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles) {
	return true;
}
