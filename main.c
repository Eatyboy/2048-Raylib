#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug true

#define FONTSIZE 86

#define BWIDTH 4
#define BHEIGHT 4
#define MAX_MERGES (BHEIGHT * BWIDTH / 2)

#define ANIMDT (0.1f)

typedef struct Pos {
	int x;
	int y;
} Pos;

typedef struct Anim {
	int dx;
	int dy;
	float t;
} Anim;

typedef struct Tile {
	int num;
	Anim *anim;
} Tile;

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
int digitCount(int n);

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	#if debug
	int boardCount = 0;
	#endif

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	Font numFont = LoadFontEx("res/AzeretMono-Bold.ttf", FONTSIZE, 0, 250);

	Color numColors[] = {
		(Color){245, 203, 192, 255},
		(Color){240, 152, 129, 255},
		(Color){240, 104, 90, 255},
		(Color){201, 32, 32, 255},
		(Color){219, 149, 15, 255},
		(Color){219, 199, 30, 255},
		(Color){106, 201, 34, 255},
		(Color){9, 156, 98, 255},
		(Color){33, 189, 201, 255},
		(Color){28, 76, 9, 255},
		(Color){117, 46, 161, 255},
		(Color){115, 3, 112, 255},
		(Color){158, 26, 91, 255},
		(Color){97, 97, 97, 255},
		(Color){59, 59, 59, 255},
		(Color){26, 26, 26, 255}
	};

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
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			newState[i][j] = (int)powf(2, 4 * i + j);
		}
	}

/*
	#if debug
	boardCount +=
	#endif
	generateTile(tiles, newState, &activeAnims);
	#if debug
	boardCount +=
	#endif
	generateTile(tiles, newState, &activeAnims);
*/

	while (!WindowShouldClose()) {
//		if (isFullBoard(tiles)) gameOver();

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
			case KEY_UP: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int i = 1; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = i-1; k >= 0; k--) {
							int collidedValue = tiles[k][j].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							for (int s = 0; s < mergeCount; ++s) {
								if (mergings[s].x == j && mergings[s].y == k) {
									mergeAhead = true;
									mergeTilesSeen++;
									break;
								}
							}

							if (mergeAhead) {
								if (mergeTilesSeen % 2 == 1) delta++;
								continue;
							}

							if (nextNum == 0) {
								nextNum = collidedValue;
							} 
							if (nextNum == value && willCombine == false) {
								delta++;
								willCombine = true;
								mergings[mergeCount] = (Pos){j, k};
								mergeCount++;
								mergings[mergeCount] = (Pos){j, i};
								mergeCount++;
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
			}
			case KEY_DOWN: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = i+1; k <= 3; ++k) {
							int collidedValue = tiles[k][j].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							for (int s = 0; s < mergeCount; ++s) {
								if (mergings[s].x == j && mergings[s].y == k) {
									mergeAhead = true;
									mergeTilesSeen++;
									break;
								}
							}

							if (mergeAhead) {
								if (mergeTilesSeen % 2 == 1) delta++;
								continue;
							}

							if (nextNum == 0) {
								nextNum = collidedValue;
							} 
							if (nextNum == value && willCombine == false) {
								delta++;
								willCombine = true;
								mergings[mergeCount] = (Pos){j, k};
								mergeCount++;
								mergings[mergeCount] = (Pos){j, i};
								mergeCount++;
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
			}
			case KEY_LEFT: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int j = 1; j < 4; ++j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = j-1; k >= 0; k--) {
							int collidedValue = tiles[i][k].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							for (int s = 0; s < mergeCount; ++s) {
								if (mergings[s].x == k && mergings[s].y == i) {
									mergeAhead = true;
									mergeTilesSeen++;
									break;
								}
							}

							if (mergeAhead) {
								if (mergeTilesSeen % 2 == 1) delta++;
								continue;
							}

							if (nextNum == 0 ) {
								nextNum = collidedValue;
							} 
							if (nextNum == value && willCombine == false) {
								delta++;
								willCombine = true;
								mergings[mergeCount] = (Pos){k, i};
								mergeCount++;
								mergings[mergeCount] = (Pos){j, i};
								mergeCount++;
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
			}
			case KEY_RIGHT: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (activeAnims > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int j = 2; j >= 0; --j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = j+1; k <= 3; ++k) {
							int collidedValue = tiles[i][k].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							for (int s = 0; s < mergeCount; ++s) {
								if (mergings[s].x == k && mergings[s].y == i) {
									mergeAhead = true;
									mergeTilesSeen++;
									break;
								}
							}

							if (mergeAhead) {
								if (mergeTilesSeen % 2 == 1) delta++;
								continue;
							}

							if (nextNum == 0) {
								nextNum = collidedValue;
							}
							if (nextNum == value && willCombine == false) {
								delta++;
								willCombine = true;
								mergings[mergeCount] = (Pos){k, i};
								mergeCount++;
								mergings[mergeCount] = (Pos){j, i};
								mergeCount++;
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
			float halfDim = 0.5f * innerDim;

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
					int num = tile.num;

					if (num == 0) continue;

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
								+ (1.0f - size) * halfDim,
							boardPos.y + thick
								+ tileY * (innerDim + thick)
								+ (1.0f - size) * halfDim,
							innerDim * size, 
							innerDim * size
						},
						0.05f, 0, numColors[(int)log2(num) - 1]
					);

					const char *numText = TextFormat("%d", num);
					int fontHalfWidth = MeasureTextEx(numFont, numText, FONTSIZE, 0).x * 0.5f;
					if (FloatEquals(size, 1.0f)) {
						DrawTextEx(numFont, numText, 
							(Vector2){
								boardPos.x + halfDim - fontHalfWidth + tileX * (innerDim + thick),
								boardPos.y + (float)FONTSIZE / 2 + tileY * (innerDim + thick),
							},
							FONTSIZE, 0, BLACK);
					}
				}
			}
			//Draw board
		EndDrawing();
	}

	UnloadFont(numFont);
	CloseWindow();

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

int digitCount(int n) {
	int count = 0;
	while (n > 0) {
		n /= 10;
		count++;
	}
	return count;
}
