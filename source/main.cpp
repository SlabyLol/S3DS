#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/stat.h>

#define GRID_WIDTH  32
#define GRID_HEIGHT 20
#define CELL_SIZE   12

struct Point {
    int x, y;
};

class SnakeGame {
private:
    C3D_RenderTarget* top = nullptr;
    C2D_TextBuf textBuf = nullptr;

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
    void update(u32 kDown, touchPosition touch);
    void draw();
    void run();
    void setTop(C3D_RenderTarget* t) { top = t; }
    void setTextBuf(C2D_TextBuf buf) { textBuf = buf; }
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
    if (!buffer) return;
    for (int i = 0; i < 2048; i++) {
        buffer[i] = (i % freq < freq/2) ? 180 : 40;
    }
    ndspWaveBuf waveBuf = {};
    waveBuf.data_vaddr = buffer;
    waveBuf.nsamples = 2048;
    ndspChnWaveBufAdd(0, &waveBuf);
}

SnakeGame::SnakeGame() {
    srand(time(NULL));
    loadHighscore();
    initGame();
}

SnakeGame::~SnakeGame() {
    saveHighscore();
    if (textBuf) C2D_TextBufDelete(textBuf);
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

void SnakeGame::update(u32 kDown, touchPosition touch) {
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

    if (kDown & KEY_TOUCH) {
        if (touch.px > 180 && dx != -1) { dx = 1; dy = 0; }
        else if (touch.px < 140 && dx != 1) { dx = -1; dy = 0; }
        if (touch.py > 140 && dy != -1) { dx = 0; dy = 1; }
        else if (touch.py < 100 && dy != 1) { dx = 0; dy = -1; }
    }

    if (kDown & KEY_RIGHT && dx != -1) { dx = 1; dy = 0; }
    if (kDown & KEY_LEFT  && dx != 1)  { dx = -1; dy = 0; }
    if (kDown & KEY_UP    && dy != 1)  { dx = 0; dy = -1; }
    if (kDown & KEY_DOWN  && dy != -1) { dx = 0; dy = 1; }

    if (frame % speed == 0) {
        Point head = {snake[0].x + dx, snake[0].y + dy};

        if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT)
            gameOver = true;

        for (int i = 1; i < snakeLength; i++) {
            if (snake[i].x == head.x && snake[i].y == head.y)
                gameOver = true;
        }

        for (int i = snakeLength - 1; i > 0; i--)
            snake[i] = snake[i-1];
        snake[0] = head;

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

    C2D_Text text;

    if (inMenu) {
        C2D_TextParse(&text, textBuf, "S3DS SNAKE");
        C2D_TextOptimize(&text);
        C2D_DrawText(&text, C2D_AlignCenter | C2D_WithColor, 200, 60, 0, 1.3f, 1.3f, C2D_Color32(0, 255, 120, 255));

        C2D_TextParse(&text, textBuf, "Drücke A zum Starten");
        C2D_TextOptimize(&text);
        C2D_DrawText(&text, C2D_AlignCenter | C2D_WithColor, 200, 130, 0, 0.75f, 0.75f, C2D_Color32(255,255,255,255));

        char hs[64];
        snprintf(hs, sizeof(hs), "Highscore: %d", highscore);
        C2D_TextParse(&text, textBuf, hs);
        C2D_TextOptimize(&text);
        C2D_DrawText(&text, C2D_AlignCenter | C2D_WithColor, 200, 170, 0, 0.7f, 0.7f, C2D_Color32(255, 215, 0, 255));
    } 
    else {
        for (int i = 0; i < snakeLength; i++) {
            u32 color = (i == 0) ? C2D_Color32(0, 255, 120, 255) : C2D_Color32(0, 200, 90, 255);
            C2D_DrawRectSolid(snake[i].x * CELL_SIZE + 8, snake[i].y * CELL_SIZE + 8, 0, CELL_SIZE-2, CELL_SIZE-2, color);
        }

        C2D_DrawRectSolid(food.x * CELL_SIZE + 8, food.y * CELL_SIZE + 8, 0, CELL_SIZE-2, CELL_SIZE-2, C2D_Color32(255, 70, 70, 255));

        char buf[80];
        snprintf(buf, sizeof(buf), "Score: %d   High: %d", score, highscore);
        C2D_TextParse(&text, textBuf, buf);
        C2D_TextOptimize(&text);
        C2D_DrawText(&text, C2D_AlignLeft | C2D_WithColor, 20, 12, 0, 0.65f, 0.65f, C2D_Color32(255,255,255,255));

        if (gameOver) {
            C2D_DrawRectSolid(70, 75, 0, 260, 90, C2D_Color32(0,0,0,200));

            C2D_TextParse(&text, textBuf, "GAME OVER");
            C2D_TextOptimize(&text);
            C2D_DrawText(&text, C2D_AlignCenter | C2D_WithColor, 200, 90, 0, 1.1f, 1.1f, C2D_Color32(255, 60, 60, 255));

            C2D_TextParse(&text, textBuf, "A = Neustart");
            C2D_TextOptimize(&text);
            C2D_DrawText(&text, C2D_AlignCenter | C2D_WithColor, 200, 125, 0, 0.7f, 0.7f, C2D_Color32(255,255,255,255));
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

        update(kDown, touch);
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
    
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    SnakeGame game;
    game.setTop(C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT));
    
    C2D_TextBuf buf = C2D_TextBufNew(4096);
    game.setTextBuf(buf);

    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/S3DS", 0777);

    game.run();

    // Cleanup
    ndspExit();           // ← Hier war der Fehler
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
