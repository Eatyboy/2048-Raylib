#include "include/raylib.h"
#include "include/raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define debug false

#define TEXT_L 86
#define TEXT_M 60
#define TEXT_S 40

#define CRIMSON (Color){165, 28, 48, 255}

#define BTN_DOWN_SCALE (0.85f)
#define BTN_HOVER_LIGHTEN (0.15f)

#define BWIDTH 4
#define BHEIGHT 4
#define MAX_MERGES (BHEIGHT * BWIDTH / 2)

#define ANIMDT (0.1f)

#define SCORE_MULT 2

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
	int score;
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
	bool isHovered;
	void (*fn)(BoardState*, GameState*);
} Button;

void restartGame(BoardState *state, GameState *gameState);
int generateTile(BoardState *state);
bool isFullBoard(Tile board[BHEIGHT][BWIDTH]);
void endAnims(BoardState *state);
void updateAnims(Tile tiles[BHEIGHT][BWIDTH], int *animCount);
Color lightenColor(Color color, float amount);
void drawButton(Button btn, Font font);
void drawCenteredText(const char *text, Rectangle parent, float fontSize, Font font, Color color, float spacing);
void handleButtons(Button* buttons, int btnCount, BoardState *state, GameState *gameState);

#if debug
bool runTests(Tile tiles[BHEIGHT][BWIDTH], int newState[BHEIGHT][BWIDTH], int *animCount, bool *spawningTiles);
void printBoard(Tile tiles[BHEIGHT][BWIDTH]);
void printTiles(Tile tiles[BHEIGHT][BWIDTH]);
void printState(int board[BHEIGHT][BWIDTH]);
int getBoardCount(int tiles[BHEIGHT][BWIDTH]);
void doNothing(BoardState *state, GameState *gameState);
#endif

int main(void) {
	const Vector2 screenSize = {1280, 720};
	const char *screenName = "2048";
	const int targetFPS = 60;

	GameState gameState = TITLESCREEN;

	InitWindow(screenSize.x, screenSize.y, screenName);
	SetTargetFPS(targetFPS);

	Font numFont = LoadFontEx("res/AzeretMono-Bold.ttf", TEXT_L, 0, 250);
	Image icon = LoadImage("res/2048logo.png");
	SetWindowIcon(icon);

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
			(Rectangle){screenSize.x / 2 - 150, screenSize.y / 2 - 80, 300, 80},
			0.2,
			GRAY,
			"Start",
			false,
			false,
			&restartGame
		}
	};

#define GO_BTN_COUNT 1
	Button gameOverButtons[GO_BTN_COUNT] = {
		(Button){
			(Rectangle){screenSize.x / 2 - 150, screenSize.y / 2 + 40, 300, 80},
			0.2,
			GRAY,
			"Restart",
			false,
			false,
			&restartGame
		}
	};

	while (!WindowShouldClose()) {
		switch (gameState) {

		case TITLESCREEN:
			handleButtons(titleScreenButtons, TS_BTN_COUNT, &state, &gameState);
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
				restartGame(&state, &gameState);
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
					endAnims(&state);
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
								if (willCombine) state.score += value * SCORE_MULT;

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
					endAnims(&state);
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
								if (willCombine) state.score += value * SCORE_MULT;

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
					endAnims(&state);
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
								if (willCombine) state.score += value * SCORE_MULT;

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
					endAnims(&state);
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
								if (willCombine) state.score += value * SCORE_MULT;

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
			handleButtons(gameOverButtons, GO_BTN_COUNT, &state, &gameState);
			break;

		default: printf("ERROR: invalid game state\n");
		}

		BeginDrawing();
			if (gameState == TITLESCREEN) {
				ClearBackground(CRIMSON);
				Rectangle titleParent = (Rectangle){0, 0, screenSize.x, screenSize.y / 3};
				drawCenteredText("2048", titleParent, 200, numFont, WHITE, 2);
				Rectangle creatorNameParent = (Rectangle){0, screenSize.y * 9/10, screenSize.x, screenSize.y / 10};
				drawCenteredText("made by Ethan Carter", creatorNameParent, TEXT_S, numFont, WHITE, 0);

				for (int i = 0; i < TS_BTN_COUNT; ++i) {
					drawButton(titleScreenButtons[i], numFont);
				}
			} else if (gameState == GAMEPLAY || gameState == GAMEOVER) {
				ClearBackground(RAYWHITE);
				//Draw Board background
				float boardDim = fminf(screenSize.x*0.8f, screenSize.y*0.8f);
				Vector2 boardPos = (Vector2){screenSize.x / 2 - boardDim / 2, screenSize.y - boardDim - 20};
				DrawRectangleRounded((Rectangle){boardPos.x, boardPos.y, boardDim, boardDim}, 0.05f, 0, BROWN);
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
							0.05f, 0, BEIGE
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
				drawCenteredText(TextFormat("Score:%d", state.score), 
					 (Rectangle){0, 0, screenSize.x, boardPos.y}, 
					 TEXT_M, numFont, BLACK, 1);
			}

			if (gameState == GAMEOVER) {
				DrawRectangleV(Vector2Zero(), screenSize, (Color){0, 0, 0, 100});

				drawCenteredText("Game Over", 
					 (Rectangle){0, 0, screenSize.x, screenSize.y * 9 / 20}, 
					 200, numFont, WHITE, 0);
				drawCenteredText(TextFormat("Your score was: %d", state.score), 
					 (Rectangle){0, screenSize.y * 8 / 20, screenSize.x, screenSize.y * 2 / 20}, 
					 TEXT_M, numFont, WHITE, 0);
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
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = i % 2; j < BWIDTH; j += 2) {
			int num = board[i][j].num;
			if (num == 0) return false;

			if (i > 0) {
				if (board[i-1][j].num == 0 
					|| board[i-1][j].num == num) 
					return false;
			}
			if (i < 3) {
				if (board[i+1][j].num == 0 
					|| board[i+1][j].num == num) 
					return false;
			}
			if (j > 0) {
				if (board[i-1][j].num == 0 
					|| board[i][j-1].num == num) 
					return false;
			}
			if (j < 3) {
				if (board[i][j+1].num == 0 
					|| board[i][j+1].num == num) 
					return false;
			}
		}
	}
	return true;
}

void endAnims(BoardState *state) {
	return;
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			if (state->board[i][j].anim == NULL) continue;
			state->board[i][j].num = state->newState[i][j];
			state->board[i][j].anim->t = 0.0f;
		}
	}

	updateAnims(state->board, &(state->animCount));
}

void restartGame(BoardState *state, GameState *gameState) {
	for (int i = 0; i < BHEIGHT; ++i) {
		for (int j = 0; j < BWIDTH; ++j) {
			state->board[i][j].num = 0;
			state->board[i][j].anim = NULL;
			state->newState[i][j] = 0;
		}
	}

	state->score = 0;
	state->animCount = 0;
	state->spawningTiles = false;

	generateTile(state);
	generateTile(state);

	*gameState = GAMEPLAY;
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

Color lightenColor(Color color, float amount) {
	return (Color){
		Lerp(color.r, 255, amount),
		Lerp(color.g, 255, amount),
		Lerp(color.b, 255, amount),
		255
	};
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
	rec = (Rectangle){btnX, btnY, btnWidth, btnHeight};

	DrawRectangleRoundedLines(rec, btn.round, 0, 3, BLACK);
	DrawRectangleRounded(rec, btn.round, 0, btn.isHovered ? lightenColor(btn.color, BTN_HOVER_LIGHTEN) : btn.color);
	drawCenteredText(btn.text, rec, fontSize, font, WHITE, 0);
}

void drawCenteredText(const char *text, Rectangle parent, float fontSize, Font font, Color color, float spacing) {
	Vector2 textHalfDim = Vector2Scale(MeasureTextEx(font, text, fontSize, 0), 0.5f);
	Vector2 textPos = (Vector2){
		parent.x + parent.width / 2 - textHalfDim.x,
		parent.y + parent.height / 2 - textHalfDim.y,
	};
	DrawTextEx(font, text, textPos, fontSize, spacing, color);
}

void handleButtons(Button* buttons, int btnCount, BoardState *state, GameState *gameState) {
	Vector2 mousePos = GetMousePosition();
	for (int i = 0; i < btnCount; ++i) {
		buttons[i].isHovered = CheckCollisionPointRec(mousePos, buttons[i].rect);
	}
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		for (int i = 0; i < btnCount; ++i) {
			buttons[i].isPressed = false;
			if (CheckCollisionPointRec(mousePos, buttons[i].rect)) {
				(buttons[i].fn)(state, gameState);
			}
		}
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		for (int i = 0; i < btnCount; ++i) {
			if (CheckCollisionPointRec(mousePos, buttons[i].rect)) {
				buttons[i].isPressed = true;
			}
		}
	}
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

void doNothing(BoardState *state, GameState *gameState) {
	return;
}
#endif
