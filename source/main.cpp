#include <3ds.h>
#include <citro2d.h>

#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>

constexpr int SCREEN_WIDTH = 400;
constexpr int SCREEN_HEIGHT = 240;

constexpr int CELL_SIZE = 10;

constexpr int GRID_WIDTH = SCREEN_WIDTH / CELL_SIZE;
constexpr int GRID_HEIGHT = SCREEN_HEIGHT / CELL_SIZE;

enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct Vec2 {
    int x;
    int y;

    bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }
};

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

std::vector<Vec2> snake;
Vec2 food;

Direction dir = RIGHT;
GameState state = MENU;

int score = 0;

void SpawnFood() {
    while (true) {
        food.x = rand() % GRID_WIDTH;
        food.y = rand() % GRID_HEIGHT;

        bool insideSnake = false;

        for (auto& s : snake) {
            if (s == food) {
                insideSnake = true;
                break;
            }
        }

        if (!insideSnake)
            break;
    }
}

void ResetGame() {
    snake.clear();

    snake.push_back({10, 10});
    snake.push_back({9, 10});
    snake.push_back({8, 10});

    dir = RIGHT;
    score = 0;

    SpawnFood();
}

void UpdateGame() {
    Vec2 head = snake.front();

    switch (dir) {
        case UP:    head.y--; break;
        case DOWN:  head.y++; break;
        case LEFT:  head.x--; break;
        case RIGHT: head.x++; break;
    }

    // Wandkollision
    if (head.x < 0 || head.x >= GRID_WIDTH ||
        head.y < 0 || head.y >= GRID_HEIGHT) {
        state = GAME_OVER;
        return;
    }

    // Selbstkollision
    for (auto& s : snake) {
        if (head == s) {
            state = GAME_OVER;
            return;
        }
    }

    snake.insert(snake.begin(), head);

    // Essen
    if (head == food) {
        score += 10;
        SpawnFood();
    } else {
        snake.pop_back();
    }
}

void DrawCell(int x, int y, u32 color) {
    C2D_DrawRectSolid(
        x * CELL_SIZE,
        y * CELL_SIZE,
        0.0f,
        CELL_SIZE - 1,
        CELL_SIZE - 1,
        color
    );
}

int main(int argc, char* argv[]) {

    srand(time(nullptr));

    gfxInitDefault();
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget* top =
        C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    ResetGame();

    u64 lastTime = osGetTime();

    while (aptMainLoop()) {

        hidScanInput();

        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_SELECT)
            break;

        // MENU
        if (state == MENU) {

            if (kDown & KEY_A) {
                ResetGame();
                state = PLAYING;
            }
        }

        // PLAYING
        else if (state == PLAYING) {

            if ((kDown & KEY_UP || kDown & KEY_DUP) && dir != DOWN)
                dir = UP;

            if ((kDown & KEY_DOWN || kDown & KEY_DDOWN) && dir != UP)
                dir = DOWN;

            if ((kDown & KEY_LEFT || kDown & KEY_DLEFT) && dir != RIGHT)
                dir = LEFT;

            if ((kDown & KEY_RIGHT || kDown & KEY_DRIGHT) && dir != LEFT)
                dir = RIGHT;

            if (kDown & KEY_START)
                state = PAUSED;

            u64 current = osGetTime();

            if (current - lastTime > 120) {
                UpdateGame();
                lastTime = current;
            }
        }

        // PAUSED
        else if (state == PAUSED) {

            if (kDown & KEY_START)
                state = PLAYING;
        }

        // GAME OVER
        else if (state == GAME_OVER) {

            if (kDown & KEY_A) {
                ResetGame();
                state = PLAYING;
            }

            if (kDown & KEY_B) {
                state = MENU;
            }
        }

        // Rendering
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(top, C2D_Color32(20, 20, 20, 255));
        C2D_SceneBegin(top);

        if (state == MENU) {

            DrawCell(0,0,C2D_Color32(0,0,0,0));

            C2D_DrawRectSolid(
                80, 50, 0,
                240, 120,
                C2D_Color32(40, 40, 40, 255)
            );

            printf("\x1b[2J");
            printf("=== SNAKE 3DS ===\n\n");
            printf("A - Start\n");
            printf("SELECT - Exit\n");
        }

        else {

            // Food
            DrawCell(food.x, food.y,
                C2D_Color32(255, 0, 0, 255));

            // Snake
            for (size_t i = 0; i < snake.size(); i++) {

                u32 color =
                    (i == 0)
                    ? C2D_Color32(0, 255, 0, 255)
                    : C2D_Color32(0, 180, 0, 255);

                DrawCell(snake[i].x, snake[i].y, color);
            }

            printf("\x1b[2J");
            printf("Score: %d\n", score);

            if (state == PAUSED) {
                printf("\nPAUSED\n");
                printf("START = Continue");
            }

            if (state == GAME_OVER) {
                printf("\nGAME OVER\n");
                printf("A = Restart\n");
                printf("B = Menu\n");
            }
        }

        C3D_FrameEnd(0);
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();

    return 0;
}
