#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug true

#define BWIDTH 4
#define BHEIGHT 4

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
void generateTile(Tile tiles[BHEIGHT][BWIDTH]);
bool isFullBoard(Tile board[BHEIGHT][BWIDTH]);
void endAnims(Tile tiles[BHEIGHT][BWIDTH]);

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	#if debug
	const float animDt = 1.0f / 100.0f;
	#else
	const float animDt = 1.0f / 10.0f;
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
	int activeAnims = 0;
	bool shouldSpawnTile = false;
	Collision collisions[BWIDTH * BHEIGHT];
	for (int i = 0; i < BWIDTH * BHEIGHT; ++i) {
		collisions[i] = NULL_COLLISION;
	}
	int activeCollisions = 0;

	generateTile(tiles);
	generateTile(tiles);

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
		#endif
		switch (input) {
			case KEY_UP:
				endAnims(tiles);
				for (int i = 1; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCollide = false;

						for (int k = i-1; k >= 0; k--) {
							int collidedValue = tiles[k][j].num;

							if (collidedValue == value) {
								delta++;
								willCollide = true;
								break;
							} else if (collidedValue != 0) {
								for (int l = k; l >= 0; l--) {
									if (tiles[l][j].num == 0) {
										delta++;
									}
								}
								delta--;
							}

							delta++;
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

						if (willCollide) {
							collisions[activeCollisions] = (Collision){j, i - delta, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_DOWN:
				endAnims(tiles);
				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCollide = false;

						for (int k = i+1; k <= 3; ++k) {
							int collidedValue = tiles[k][j].num;

							if (collidedValue == value) {
								delta++;
								willCollide = true;
								break;
							} else if (collidedValue != 0) {
								for (int l = k; l <= 3; l++) {
									if (tiles[l][j].num == 0) {
										delta++;
									}
								}
								delta--;
							}

							delta++;
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

						if (willCollide) {
							collisions[activeCollisions] = (Collision){j, i + delta, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_LEFT:
				endAnims(tiles);
				for (int j = 1; j < 4; ++j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCollide = false;

						for (int k = j-1; k >= 0; k--) {
							int collidedValue = tiles[i][k].num;

							if (collidedValue == value) {
								delta++;
								willCollide = true;
								break;
							} else if (collidedValue != 0) {
								for (int l = k; l >= 0; l--) {
									if (tiles[i][l].num == 0) {
										delta++;
									}
								}
								delta--;
							}

							delta++;
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

						if (willCollide) {
							collisions[activeCollisions] = (Collision){j - delta, i, value + value};
							activeCollisions++;
						}
					}
				}
				break;
			case KEY_RIGHT:
				endAnims(tiles);
				for (int j = 2; j >= 0; --j) {
					for (int i = 0; i < 4; ++i) {
						int value = tiles[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCollide = false;

						for (int k = j+1; k <= 3; ++k) {
							int collidedValue = tiles[i][k].num;

							if (collidedValue == value) {
								delta++;
								willCollide = true;
								break;
							} else if (collidedValue != 0) {
								for (int l = k; l <= 3; l++) {
									if (tiles[i][l].num == 0) {
										delta++;
									}
								}
								delta--;
							}

							delta++;
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

						if (willCollide) {
							collisions[activeCollisions] = (Collision){j + delta, i, value + value};
							activeCollisions++;
						}
					}
				}
				break;
		}

		if (activeAnims > 0) {
			for (int i = 0; i < BHEIGHT; ++i) {
				for (int j = 0; j < BWIDTH; ++j) {
					Anim *anim = tiles[i][j].anim;
					if (anim == NULL) continue;
					if (!FloatEquals(anim->t, 0.0f)) {
						anim->t = fmaxf(anim->t - animDt, 0.0f);
					} else {
						int newX = j + anim->dx;
						int newY = i + anim->dy;
						bool isCollision = false;

						for (int k = 0; k < activeCollisions; ++k) {
							Collision collision = collisions[k];
							if (collision.x == newX && collision.y == newY) {
								isCollision = true;
								for (int l = k; l < activeCollisions - 1; ++l) {
									collisions[l] = collisions[l+1];
								}
								activeCollisions--;
								collisions[activeCollisions] = NULL_COLLISION;
								break;
							}
						}

						if (isCollision) {
							#if debug
							printf("Collisions: ");
							#endif
							for (size_t i = 0; i < activeCollisions; i++) {
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

						free(anim);
						activeAnims--;
					}
				}
			}
		}

		if (shouldSpawnTile && activeAnims == 0) {
			generateTile(tiles);
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

					DrawRectangleRounded(
						(Rectangle){
							boardPos.x + thick + tileX * (innerDim + thick),
							boardPos.y + thick + tileY * (innerDim + thick),
							innerDim, 
							innerDim
						},
						0.05f, 0, numColors[(int)log2(tile.num) - 1]
					);
					DrawText(TextFormat("%d", tile.num), 
						boardPos.x + 50 + tileX * (innerDim + thick),
						boardPos.y + 30 + tileY * (innerDim + thick),
						80, BLACK);
				}
			}
			//Draw board
		EndDrawing();
	}
	return 0;
}

void generateTile(Tile tiles[BHEIGHT][BWIDTH]) {
	int possibleNums[] = {2,4};
	int newIndex;
	do {
		newIndex = GetRandomValue(0, 15);
	} while (tiles[newIndex % BHEIGHT][newIndex / BWIDTH].num != 0);
	int newNum = possibleNums[GetRandomValue(0,1)];
	tiles[newIndex % BHEIGHT][newIndex / BWIDTH] = (Tile){
		newNum,
		NULL
	};
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

void endAnims(Tile tiles[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			if (tiles[i][j].anim == NULL) continue;
			tiles[i][j].anim->t = 0.0f;
		}
	}
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

	generateTile(board);
	generateTile(board);
}
