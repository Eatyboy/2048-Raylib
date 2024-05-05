#include "include/raylib.h"
#include <math.h>

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

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(RAYWHITE);
			//Draw Board
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
		EndDrawing();
	}
	return 0;
}
