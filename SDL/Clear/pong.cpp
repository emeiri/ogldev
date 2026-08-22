/*
 * This example code $WHAT_IT_DOES.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <algorithm>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define PADDLE_OFFSET 25.0f

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

class GameClock {

public:

    GameClock() {}

    void Init()
    {
        m_tickCount = SDL_GetTicks();
    }

    float GetDeltaTime()
    {
        Uint64 CurrentTick = SDL_GetTicks();
        float DeltaTime = (CurrentTick - m_tickCount) / 1000.0f; // Convert to seconds
        m_tickCount = CurrentTick;

        if (DeltaTime > 0.1f) {
            DeltaTime = 0.1f; // Clamp to 100ms to avoid large jumps
        }

        return DeltaTime;
    }

private:
    Uint64 m_tickCount = 0;
};



float BallSize = 20.0f;
float HalfBallSize = BallSize / 2.0f;
Vec2 BallPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
Vec2 PaddlePosL = { PADDLE_OFFSET, WINDOW_HEIGHT / 2.0f };
GameClock Clock;
Vec2 PaddlePosR = { WINDOW_WIDTH - PADDLE_OFFSET, WINDOW_HEIGHT / 2.0f };
float PaddleWidth = 30.0f;
float PaddleHeight = 300.0f;
float HalfPaddleHeight = PaddleHeight / 2.0f;
Uint64 TickCount = 0;
Vec2 BallVelocity = { -200.0f, 235.0f }; // pixels per second


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Example HUMAN READABLE NAME", "1.0", "com.example.CATEGORY-NAME");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL initialized");

    if (!SDL_CreateWindowAndRenderer("examples/CATEGORY/NAME", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_SetRenderVSync(renderer, 1);

    TickCount = SDL_GetTicks();

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if ((event->type == SDL_EVENT_KEY_DOWN) && 
        (event->key.scancode == SDL_SCANCODE_ESCAPE)) {
        SDL_Log("Escape key pressed, quitting");
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
   
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}


/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    float DeltaTime = Clock.GetDeltaTime();

   // printf("DeltaTime: %f\n", DeltaTime);
    SDL_SetRenderDrawColor(renderer, 16, 16, 16, 255);
    SDL_RenderClear(renderer);

    int NumKeys = 0;
    const bool* pKeys = SDL_GetKeyboardState(&NumKeys);

    float PaddleSpeed = 600.0f; // Pixels per second
    float CurrentPaddleVelocity = 0.0f;

    if (pKeys[SDL_SCANCODE_W]) {
        CurrentPaddleVelocity -= PaddleSpeed;
    }
    if (pKeys[SDL_SCANCODE_S]) {
        CurrentPaddleVelocity += PaddleSpeed;
    }

    // 4. Update Paddle Positions via Time Delta
    PaddlePosL.y += CurrentPaddleVelocity * DeltaTime;
    
    // 5. Clamp Paddle within Window Constraints safely
    if (PaddlePosL.y - HalfPaddleHeight < 0.0f) {
        PaddlePosL.y = HalfPaddleHeight;
    } else if (PaddlePosL.y + HalfPaddleHeight > WINDOW_HEIGHT) {
        PaddlePosL.y = WINDOW_HEIGHT - HalfPaddleHeight;
    }

    // 6. Update Ball Mechanics via Time Delta
    BallPos.x += BallVelocity.x * DeltaTime;
    BallPos.y += BallVelocity.y * DeltaTime;

    // 7. Ball Ceiling / Floor Boundaries Collisions
    if ((BallPos.y - HalfBallSize <= 0.0f && BallVelocity.y < 0.0f) ||
        (BallPos.y + HalfBallSize >= WINDOW_HEIGHT && BallVelocity.y > 0.0f)) {
        BallVelocity.y = -BallVelocity.y;
    }

    // 8. Ball vs Paddle Precise Rect Collision Detection
    bool CollideWithPaddle =
        (BallPos.x - HalfBallSize <= PaddlePosL.x + PaddleWidth / 2.0f) &&
        (BallPos.x + HalfBallSize >= PaddlePosL.x - PaddleWidth / 2.0f) &&
        (BallPos.y + HalfBallSize >= PaddlePosL.y - HalfPaddleHeight) &&
        (BallPos.y - HalfBallSize <= PaddlePosL.y + HalfPaddleHeight);

    if (CollideWithPaddle && BallVelocity.x < 0.0f) {
        BallVelocity.x = -BallVelocity.x;
        // Optional: Slightly boost speed upon impact to increase difficulty
        BallVelocity.x *= 1.05f;
        BallVelocity.y *= 1.05f;
    }

    // 9. Reset Ball if it goes out of bounds (Left Wall Point Loss)
    if (BallPos.x < 0.0f) {
        BallPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
        BallVelocity = { -200.0f, 235.0f }; // Reset speed
    }
    // Bounce off right wall for now since AI/Right paddle isn't written yet
    if (BallPos.x + HalfBallSize >= WINDOW_WIDTH && BallVelocity.x > 0.0f) {
        BallVelocity.x = -BallVelocity.x;
    }

    // 10. Render Geometries to screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_FRect PaddleRect{ PaddlePosL.x - PaddleWidth / 2.0f, PaddlePosL.y - HalfPaddleHeight, PaddleWidth, PaddleHeight };
    SDL_RenderFillRect(renderer, &PaddleRect);

    SDL_FRect BallRect{ BallPos.x - HalfBallSize, BallPos.y - HalfBallSize, BallSize, BallSize };
    SDL_RenderFillRect(renderer, &BallRect);
    SDL_RenderPresent(renderer);    
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

