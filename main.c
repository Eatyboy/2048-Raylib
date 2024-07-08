#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug true

#define TEXT_L 86
#define TEXT_M 60
#define TEXT_S 40

#define BTN_DOWN_SCALE (0.85f)

#define BWIDTH 4
#define BHEIGHT 4
#define MAX_MERGES (BHEIGHT * BWIDTH / 2)

#define ANIMDT (0.1f)

typedef enum GameState {
	TITLESCREEN,
	GAMEPLAY,
	GAMEOVER
} GameState;

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

typedef struct BoardState {
	Tile board[BHEIGHT][BWIDTH];
	int newState[BHEIGHT][BWIDTH];
	int animCount;
	bool spawningTiles;
} BoardState;

typedef struct Button {
	Rectangle rect;
	float round;
	Color color;
	const char* text;
	bool isPressed;
	void (*fn)(BoardState*);
} Button;

void restartGame(BoardState *state);
int generateTile(BoardState *state);
bool isFullBoard(Tile board[BHEIGHT][BWIDTH]);
void endAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
void updateAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
int digitCount(int n);
void drawButton(Button btn, Font font);

#if debug
bool runTests(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles);
void printBoard(Tile tiles[BHEIGHT][BWIDTH]);
void printTiles(Tile tiles[BHEIGHT][BWIDTH]);
void printState(int board[BHEIGHT][BWIDTH]);
int getBoardCount(int tiles[BHEIGHT][BWIDTH]);
#endif

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	GameState gameState = GAMEPLAY;

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	Font numFont = LoadFontEx("res/AzeretMono-Bold.ttf", TEXT_L, 0, 250);

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

	BoardState state;

#define TS_BTN_COUNT 1
	Button titleScreenButtons[TS_BTN_COUNT] = {
		(Button){
			(Rectangle){screenSize.x / 2, screenSize.y / 2, 300, 80},
			0.2,
			GRAY,
			"Start",
			false,
			&restartGame
		}
	};

#define GO_BTN_COUNT 1
	Button gameOverButtons[GO_BTN_COUNT] = {
		(Button){
			(Rectangle){screenSize.x / 2 - 150, screenSize.y / 2 - 80, 300, 80},
			0.2,
			GRAY,
			"Restart",
			false,
			&restartGame
		}
	};

	restartGame(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);
	generateTile(&state);

	while (!WindowShouldClose()) {
		switch (gameState) {

		case TITLESCREEN:
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 mousePos = GetMousePosition();
			for (int i = 0; i < TS_BTN_COUNT; ++i) {
				if (CheckCollisionPointRec(mousePos, titleScreenButtons[i].rect)) {
					(titleScreenButtons[i].fn)(&state);
				}
			}
		}
		break;

		case GAMEPLAY:
		if (isFullBoard(state.board) && state.animCount == 0) gameState = GAMEOVER;

		int input = GetKeyPressed();
		#if debug
		if (input == KEY_SPACE) {
			printBoard(state.board);
		}
		if (input == KEY_B) {
			printTiles(state.board);
		}
		if (input == KEY_R) {
			restartGame(&state);
			printf("Restarted game\n");
		}
		if (input == KEY_I) {
			printf("Active anims: %d, shouldSpawnTile: %s\n", state.animCount, state.spawningTiles ? "true" : "false");
			if (state.animCount > 0) {
				for (int i = 0; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						Anim *a = state.board[i][j].anim;
						if (a == NULL) printf("Anim at (%d, %d) is null\n", j, i);
						else printf("Anim at (%d, %d) has dx: %d, dy: %d, t: %f\n", j, i, a->dx, a->dy, a->t);
					}
				}
			}
		}
		int prevState[BHEIGHT][BWIDTH];
		for (int i = 0; i < BHEIGHT; ++i) {
			for (int j = 0; j < BWIDTH; ++j) {
				prevState[i][j] = state.board[i][j].num;
			}
		}
		#endif
		switch (input) {
			case KEY_UP: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (state.animCount > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int i = 1; i < BHEIGHT; ++i) {
					for (int j = 0; j < BWIDTH; ++j) {
						int value = state.board[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = i-1; k >= 0; k--) {
							int collidedValue = state.board[k][j].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							} else {
								nextNum = -1;
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
							state.newState[i][j] = 0;
							state.newState[i - delta][j] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = -delta;
							newAnim->t = 1;

							state.board[i][j].anim = newAnim;
							state.animCount++;

							state.spawningTiles = true;
						}
					}
				}
				break;
			}
			case KEY_DOWN: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (state.animCount > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int i = 2; i >= 0; --i) {
					for (int j = 0; j < 4; ++j) {
						int value = state.board[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = i+1; k <= 3; ++k) {
							int collidedValue = state.board[k][j].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							} else {
								nextNum = -1;
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
							state.newState[i][j] = 0;
							state.newState[i + delta][j] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = 0;
							newAnim->dy = delta;
							newAnim->t = 1;

							state.board[i][j].anim = newAnim;
							state.animCount++;

							state.spawningTiles = true;
						}
					}
				}
				break;
			}
			case KEY_LEFT: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (state.animCount > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int j = 1; j < 4; ++j) {
					for (int i = 0; i < 4; ++i) {
						int value = state.board[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = j-1; k >= 0; k--) {
							int collidedValue = state.board[i][k].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0 ) {
								nextNum = collidedValue;
							} else {
								nextNum = -1;
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
							state.newState[i][j] = 0;
							state.newState[i][j - delta] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = -delta;
							newAnim->dy = 0;
							newAnim->t = 1;

							state.board[i][j].anim = newAnim;
							state.animCount++;

							state.spawningTiles = true;
						}
					}
				}
				break;
			}
			case KEY_RIGHT: {
				//endAnims(tiles, collisions, &activeAnims, &activeAnims);
				if (state.animCount > 0) break;

				Pos mergings[MAX_MERGES];
				for (int i = 0; i < MAX_MERGES; ++i) {
					mergings[i] = (Pos){-1, -1};
				}
				int mergeCount = 0;

				for (int j = 2; j >= 0; --j) {
					for (int i = 0; i < 4; ++i) {
						int value = state.board[i][j].num;

						if (value == 0) continue;

						int delta = 0;
						bool willCombine = false;
						int nextNum = 0;
						int mergeTilesSeen = 0;

						for (int k = j+1; k <= 3; ++k) {
							int collidedValue = state.board[i][k].num;
							bool mergeAhead = false;

							if (collidedValue == 0) {
								delta++;
								continue;
							} 

							if (nextNum == 0) {
								nextNum = collidedValue;
							} else {
								nextNum = -1;
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
							state.newState[i][j] = 0;
							state.newState[i][j + delta] = willCombine ? value + value : value;

							Anim *newAnim = malloc(sizeof(Anim));
							newAnim->dx = delta;
							newAnim->dy = 0;
							newAnim->t = 1;

							state.board[i][j].anim = newAnim;
							state.animCount++;

							state.spawningTiles = true;
						}
					}
				}
				break;
			}
		}

		updateAnims(state.board, &(state.animCount));

		if (state.animCount == 0) {
			for (int i = 0; i < BHEIGHT; ++i) {
				for (int j = 0; j < BWIDTH; ++j) {
					state.board[i][j].num = state.newState[i][j];
				}
			}

			if (state.spawningTiles) {
				generateTile(&state);
			}
		}
		break;

		case GAMEOVER:
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			for (int i = 0; i < GO_BTN_COUNT; ++i) {
				gameOverButtons[i].isPressed = false;
			}
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Vector2 mousePos = GetMousePosition();
			for (int i = 0; i < GO_BTN_COUNT; ++i) {
				if (CheckCollisionPointRec(mousePos, gameOverButtons[i].rect)) {
					gameOverButtons[i].isPressed = true;
					(gameOverButtons[i].fn)(&state);
				}
			}
		}
		break;

		default: printf("ERROR: invalid game state\n");
		}

		BeginDrawing();
			if (gameState == TITLESCREEN) {
				for (int i = 0; i < TS_BTN_COUNT; ++i) {
					Button button = titleScreenButtons[i];
					DrawRectangleRounded(button.rect, button.round, 0, button.color);
				}
			} else if (gameState == GAMEPLAY || gameState == GAMEOVER) {
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
					Tile tile = state.board[i][j];
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

					int textSize = TEXT_L;
					const char *numText = TextFormat("%d", num);
					Vector2 fontHalfDim = Vector2Scale(MeasureTextEx(numFont, numText, TEXT_L, 0), 0.5f);
					if (fontHalfDim.x * 2 > innerDim - thick) {
						textSize = TEXT_M;
						fontHalfDim = Vector2Scale(MeasureTextEx(numFont, numText, TEXT_M, 0), 0.5f);
					}
					if (fontHalfDim.x * 2 > innerDim - thick) {
						textSize = TEXT_S;
						fontHalfDim = Vector2Scale(MeasureTextEx(numFont, numText, TEXT_S, 0), 0.5f);
					}
					if (FloatEquals(size, 1.0f)) {
						DrawTextEx(numFont, numText, 
							(Vector2){
								boardPos.x + thick + halfDim - fontHalfDim.x + tileX * (innerDim + thick),
								boardPos.y + thick + halfDim - fontHalfDim.y + tileY * (innerDim + thick),
							},
							textSize, 0, BLACK);
					}
				}
			}
			}

			if (gameState == GAMEOVER) {
				DrawRectangleV(Vector2Zero(), screenSize, (Color){0, 0, 0, 100});
				for (int i = 0; i < GO_BTN_COUNT; ++i) {
					drawButton(gameOverButtons[i], numFont);
				}
			}
		EndDrawing();
	}

	UnloadFont(numFont);
	CloseWindow();

	return 0;
}

int generateTile(BoardState *state) {
	int possibleNums[] = {2,4};
	int newIndex;
	do {
		newIndex = GetRandomValue(0, 15);
	} while (state->board[newIndex % BHEIGHT][newIndex / BWIDTH].num != 0);

	int newNum = possibleNums[GetRandomValue(0,1)];
	Anim *newAnim = malloc(sizeof(Anim));
	newAnim->dx = 0;
	newAnim->dy = 0;
	newAnim->t = 1;
	state->board[newIndex % BHEIGHT][newIndex / BWIDTH].anim = newAnim;
	state->board[newIndex % BHEIGHT][newIndex / BWIDTH].num = newNum;
	state->newState[newIndex % BHEIGHT][newIndex / BWIDTH] = newNum;
	state->animCount++;

	state->spawningTiles = false;

	return newNum;
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

void restartGame(BoardState *state) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			state->board[i][j].num = 0;
			state->board[i][j].anim = NULL;
			state->newState[i][j] = 0;
		}
	}

	state->animCount = 0;
	state->spawningTiles = false;

	generateTile(state);
	generateTile(state);
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

int digitCount(int n) {
	int count = 0;
	while (n > 0) {
		n /= 10;
		count++;
	}
	return count;
}

void drawButton(Button btn, Font font) {
	Rectangle rec = btn.rect;
	bool btnDown = btn.isPressed;
	float sizeDiffMult = (1 - BTN_DOWN_SCALE) / 2;

	float btnX = rec.x; 
	float btnY = rec.y;
	float btnWidth = rec.width; 
	float btnHeight = rec.height;
	float fontSize = TEXT_M;
	if (btnDown) {
		btnX += rec.width * sizeDiffMult;
		btnY += rec.height * sizeDiffMult;
		btnWidth *= BTN_DOWN_SCALE;
		btnHeight *= BTN_DOWN_SCALE;
		fontSize *= BTN_DOWN_SCALE;
	}
	Vector2 textDim = MeasureTextEx(font, btn.text, fontSize, 0);
	float txtX = btnX + btnWidth / 2 - textDim.x / 2;
	float txtY = btnY + btnHeight / 2 - textDim.y / 2;

	DrawRectangleRoundedLines((Rectangle){btnX, btnY, btnWidth, btnHeight}, btn.round, 0, 3, BLACK);
	DrawRectangleRounded((Rectangle){btnX, btnY, btnWidth, btnHeight}, btn.round, 0, btn.color);
	DrawTextEx(font, btn.text, (Vector2){txtX, txtY}, fontSize, 0, WHITE);
}

#if debug
void printBoard(Tile tiles[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			printf("%d ", tiles[i][j].num);
		}
		printf("\n");
	}
	printf("---------\n");
}

void printState(int board[BHEIGHT][BWIDTH]) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			printf("%d ", board[i][j]);
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

bool runTests(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles) {
	return true;
}

int getBoardCount(int tiles[BHEIGHT][BWIDTH]) {
	int count = 0;
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			count += tiles[i][j];
		}
	}
	return count;
}
#endif
