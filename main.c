#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug true

#define BWIDTH 4
#define BHEIGHT 4

#define ANIMDT (0.01f)

typedef struct Anim {
	int dx;
	int dy;
	float t;
} Anim;

typedef struct Tile {
	int num;
	Anim *anim;
} Tile;

typedef struct Collision {
	int x;
	int y;
	int value;
} Collision;

#define NULL_COLLISION (Collision){-1, -1, 0}

Color numColors[] = {
	(Color){220, 150, 150, 255},
	(Color){220, 125, 125, 255},
	(Color){220, 100, 100, 255},
	(Color){235, 75, 75, 255}
};

void gameOver();
void restartGame(Tile board[BHEIGHT][BWIDTH], 
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount,
				bool *spawningTiles);
void printTiles(Tile tiles[BHEIGHT][BWIDTH]);
void generateTile(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
bool isFullBoard(Tile board[BHEIGHT][BWIDTH]);
void endAnims(Tile tiles[BHEIGHT][BWIDTH],
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount);
void updateAnims(Tile tiles[BHEIGHT][BWIDTH],
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount);

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

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
	int activeAnims = 0;
	bool shouldSpawnTile = false;
	Collision collisions[BWIDTH * BHEIGHT];
	for (int i = 0; i < BWIDTH * BHEIGHT; ++i) {
		collisions[i] = NULL_COLLISION;
	}
	int activeCollisions = 0;

	generateTile(tiles, &activeAnims);
	generateTile(tiles, &activeAnims);

	while (!WindowShouldClose()) {
		if (isFullBoard(tiles)) gameOver();

		int input = GetKeyPressed();
		#if debug
		if (input == KEY_SPACE) {
			printTiles(tiles);
		}
		if (input == KEY_R) {
			restartGame(tiles, collisions, &activeAnims, &activeCollisions, &shouldSpawnTile);
			printf("Restarted game\n");
		}
		if (input == KEY_I) {
			printf("Active anims: %d, Active Collisions: %d, shouldSpawnTile: %s\n", activeAnims, activeCollisions, shouldSpawnTile ? "true" : "false");
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
		#endif
		switch (input) {
			case KEY_UP:
				endAnims(tiles, collisions, &activeAnims, &activeAnims);
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
							} else if (nextNum == collidedValue) {
								delta++;
								nextNum = -1;
							}

							if (collidedValue == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = -delta;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
							shouldSpawnTile = true;
						}

						if (willCombine) {
							collisions[activeCollisions] = (Collision){j, i - delta, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_DOWN:
				endAnims(tiles, collisions, &activeAnims, &activeAnims);
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
							} else if (nextNum == collidedValue) {
								delta++;
								nextNum = -1;
							}

							if (collidedValue == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = delta;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
							shouldSpawnTile = true;
						}

						if (willCombine) {
							collisions[activeCollisions] = (Collision){j, i + delta, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_LEFT:
				endAnims(tiles, collisions, &activeAnims, &activeAnims);
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
							} else if (nextNum == collidedValue) {
								delta++;
								nextNum = -1;
							}

							if (collidedValue == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = -delta;
							newAnim->dy = 0;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
							shouldSpawnTile = true;
						}

						if (willCombine) {
							collisions[activeCollisions] = (Collision){j - delta, i, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_RIGHT:
				endAnims(tiles, collisions, &activeAnims, &activeAnims);
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
							} else if (nextNum == collidedValue) {
								delta++;
								nextNum = -1;
							}

							if (collidedValue == value) {
								delta++;
								willCombine = true;
							}
						}

						if (delta > 0) {
							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = delta;
							newAnim->dy = 0;
							newAnim->t = 1;
							tiles[i][j].anim = newAnim;
							activeAnims++;
							shouldSpawnTile = true;
						}

						if (willCombine) {
							collisions[activeCollisions] = (Collision){j + delta, i, value + value};
							activeCollisions++;
						}
					}
				}
				break;
		}

		updateAnims(tiles, collisions, &activeAnims, &activeCollisions);

		if (shouldSpawnTile && activeAnims == 0) {
			generateTile(tiles, &activeAnims);
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

void generateTile(Tile tiles[BHEIGHT][BWIDTH], int *animCount) {
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
	tiles[newIndex % BHEIGHT][newIndex / BWIDTH] = (Tile){
		newNum,
		newAnim
	};
	(*animCount)++;
}

void printTiles(Tile tiles[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			printf("%d ", tiles[i][j].num);
		}
		printf("\n");
	}
	printf("---------\n");
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

void endAnims(Tile tiles[BHEIGHT][BWIDTH],
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount) {
	printf("Active anims before: %d\n", *animCount);
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			if (tiles[i][j].anim == NULL) continue;
			tiles[i][j].anim->t = 0.0f;
			Anim *a = tiles[i][j].anim;
			printf("ended anim at (%d, %d) going to (%d, %d)\n", j, i, j + a->dx, i + a-> dy);
		}
	}

	updateAnims(tiles, collisions, animCount, collisionCount);
	printf("Active anims after: %d\n", *animCount);
}

void restartGame(Tile board[BHEIGHT][BWIDTH], 
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount,
				bool *spawningTiles) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			board[i][j].num = 0;
			board[i][j].anim = NULL;
		}
	}

	for (int i = 0; i < BHEIGHT * BWIDTH; ++i) {
		collisions[i] = NULL_COLLISION;
	}

	animCount = 0;
	collisionCount = 0;
	spawningTiles = false;

	generateTile(board, animCount);
	generateTile(board, animCount);
}

void updateAnims(Tile tiles[BHEIGHT][BWIDTH],
				Collision collisions[BHEIGHT * BWIDTH],
				int *animCount,
				int *collisionCount) {
	if (*animCount <= 0) return;

	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			Anim *anim = tiles[i][j].anim;
			if (anim == NULL) continue;
			if (!FloatEquals(anim->t, 0.0f)) {
				anim->t = fmaxf(anim->t - ANIMDT, 0.0f);
			} else {
				if (anim->dx == 0 && anim->dy == 0) {
					tiles[i][j].anim = NULL;
					#if debug
					printf("Tile at (%d, %d) spawned\n", j, i);
					#endif
				} else {
					int newX = j + anim->dx;
					int newY = i + anim->dy;
					bool isCollision = false;

					for (int k = 0; k < *collisionCount; ++k) {
						Collision collision = collisions[k];
						if (collision.x == newX && collision.y == newY) {
							isCollision = true;
							for (int l = k; l < *collisionCount - 1; ++l) {
								collisions[l] = collisions[l+1];
							}
							(*collisionCount)--;
							collisions[*collisionCount] = NULL_COLLISION;
							break;
						}
					}

					if (isCollision) {
						#if debug
						printf("Collisions: ");
						#endif
						for (size_t i = 0; i < *collisionCount; i++) {
							Collision c = collisions[i];
							printf("(%d, %d)", c.x, c.y);
						}
						#if debug
						printf("\ncollision at %d, %d\n", newX, newY);
						#endif
						tiles[newY][newX].num *= 2;
						tiles[i][j].num = 0;
						tiles[i][j].anim = NULL;
					} else {
						tiles[i][j].anim = NULL;
						tiles[newY][newX] = tiles[i][j];
						tiles[i][j].num = 0;
						#if debug
						printf("Tile at (%d, %d) moved to (%d, %d)\n", j, i, newX, newY);
						#endif
					}
				}

				free(anim);
				(*animCount)--;
			}
		}
	}
}
