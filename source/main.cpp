#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>

#define GRID_WIDTH  32
#define GRID_HEIGHT 20
#define CELL_SIZE   12

struct Point {
    int x, y;
};

class SnakeGame {
private:
    C3D_RenderTarget* top = nullptr;
    Point snake[400];
    int snakeLength = 3;
    Point food;
    int dx = 1, dy = 0;
    int score = 0;
    int highscore = 0;
    bool gameOver = false;
    bool inMenu = true;
    u32 frame = 0;
    u32 speed = 8;

    void loadHighscore();
    void saveHighscore();
    void playSound(int freq);

public:
    SnakeGame();
    ~SnakeGame();
    void initGame();
    void update(u32 kDown, u32 kHeld, touchPosition touch);
    void draw();
    void run();
};

void SnakeGame::loadHighscore() {
    FILE* f = fopen("sdmc:/3ds/S3DS/highscore.txt", "r");
    if (f) {
        fscanf(f, "%d", &highscore);
        fclose(f);
    }
}

void SnakeGame::saveHighscore() {
    if (score > highscore) highscore = score;
    FILE* f = fopen("sdmc:/3ds/S3DS/highscore.txt", "w");
    if (f) {
        fprintf(f, "%d", highscore);
        fclose(f);
    }
}

void SnakeGame::playSound(int freq) {
    u8* buffer = (u8*)linearAlloc(4096);
    for (int i = 0; i < 2048; i++) {
        buffer[i] = (i % freq < freq/2) ? 180 : 40;
    }
    ndspWaveBuf waveBuf = {};
    waveBuf.data_vaddr = buffer;
    waveBuf.nsamples = 2048;
    NDSP_ChnWaveBufAdd(0, &waveBuf);
}

SnakeGame::SnakeGame() {
    srand(time(NULL));
    loadHighscore();
    initGame();
}

SnakeGame::~SnakeGame() {
    saveHighscore();
}

void SnakeGame::initGame() {
    snake[0] = {10, 10};
    snake[1] = {9, 10};
    snake[2] = {8, 10};
    snakeLength = 3;
    dx = 1; dy = 0;
    score = 0;
    gameOver = false;
    speed = 8;
    food = {rand() % (GRID_WIDTH-2) + 1, rand() % (GRID_HEIGHT-2) + 1};
}

void SnakeGame::update(u32 kDown, u32 kHeld, touchPosition touch) {
    if (inMenu) {
        if (kDown & KEY_A) {
            inMenu = false;
            initGame();
        }
        return;
    }

    if (gameOver) {
        if (kDown & KEY_A) {
            saveHighscore();
            initGame();
        }
        return;
    }

    // Touchscreen
    if (kDown & KEY_TOUCH) {
        if (touch.px > 180 && dx != -1) { dx = 1; dy = 0; }
        else if (touch.px < 140 && dx != 1) { dx = -1; dy = 0; }
        if (touch.py > 140 && dy != -1) { dx = 0; dy = 1; }
        else if (touch.py < 100 && dy != 1) { dx = 0; dy = -1; }
    }

    // D-Pad
    if (kDown & KEY_RIGHT && dx != -1) { dx = 1; dy = 0; }
    if (kDown & KEY_LEFT  && dx !=  1) { dx = -1; dy = 0; }
    if (kDown & KEY_UP    && dy !=  1) { dx = 0; dy = -1; }
    if (kDown & KEY_DOWN  && dy != -1) { dx = 0; dy = 1; }

    if (frame % speed == 0) {
        Point head = {snake[0].x + dx, snake[0].y + dy};

        // Kollision
        if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT)
            gameOver = true;

        for (int i = 1; i < snakeLength; i++) {
            if (snake[i].x == head.x && snake[i].y == head.y)
                gameOver = true;
        }

        // Schlange bewegen
        for (int i = snakeLength - 1; i > 0; i--)
            snake[i] = snake[i-1];
        snake[0] = head;

        // Futter
        if (snake[0].x == food.x && snake[0].y == food.y) {
            score += 10;
            snakeLength++;
            if (speed > 3) speed--;
            food = {rand() % (GRID_WIDTH-2) + 1, rand() % (GRID_HEIGHT-2) + 1};
            playSound(8);
        } else {
            playSound(25);
        }
    }
    frame++;
}

void SnakeGame::draw() {
    C2D_TargetClear(top, C2D_Color32(20, 20, 30, 255));
    C2D_SceneBegin(top);

    if (inMenu) {
        C2D_DrawText("S3DS SNAKE", C2D_AlignCenter, 200, 60, 0, 1.3f, 1.3f, C2D_Color32(0, 255, 120, 255));
        C2D_DrawText("Drücke A zum Starten", C2D_AlignCenter, 200, 130, 0, 0.75f, 0.75f, C2D_Color32(255,255,255,255));
        char hs[64];
        snprintf(hs, sizeof(hs), "Highscore: %d", highscore);
        C2D_DrawText(hs, C2D_AlignCenter, 200, 170, 0, 0.7f, 0.7f, C2D_Color32(255, 215, 0, 255));
    } 
    else {
        // Schlange zeichnen
        for (int i = 0; i < snakeLength; i++) {
            u32 color = (i == 0) ? C2D_Color32(0, 255, 120, 255) : C2D_Color32(0, 200, 90, 255);
            C2D_DrawRectSolid(snake[i].x * CELL_SIZE + 8, snake[i].y * CELL_SIZE + 8, 0, CELL_SIZE-2, CELL_SIZE-2, color);
        }

        // Futter
        C2D_DrawRectSolid(food.x * CELL_SIZE + 8, food.y * CELL_SIZE + 8, 0, CELL_SIZE-2, CELL_SIZE-2, C2D_Color32(255, 70, 70, 255));

        // Score
        char buf[80];
        snprintf(buf, sizeof(buf), "Score: %d   High: %d", score, highscore);
        C2D_DrawText(buf, C2D_AlignLeft, 20, 12, 0, 0.65f, 0.65f, C2D_Color32(255,255,255,255));

        if (gameOver) {
            C2D_DrawRectSolid(70, 75, 0, 260, 90, C2D_Color32(0,0,0,200));
            C2D_DrawText("GAME OVER", C2D_AlignCenter, 200, 90, 0, 1.1f, 1.1f, C2D_Color32(255, 60, 60, 255));
            C2D_DrawText("A = Neustart", C2D_AlignCenter, 200, 125, 0, 0.7f, 0.7f, C2D_Color32(255,255,255,255));
        }
    }
}

void SnakeGame::run() {
    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;
        hidTouchRead(&touch);
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        update(kDown, hidKeysHeld(), touch);
        draw();

        C3D_FrameEnd(0);
    }
}

int main(int argc, char** argv) {
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    NDSP_Init();
    NDSP_SetOutputMode(NDSP_OUTPUT_STEREO);

    SnakeGame game;
    game.top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/S3DS", 0777);

    game.run();

    NDSP_Fini();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
