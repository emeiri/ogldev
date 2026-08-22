/*
 * This example code $WHAT_IT_DOES.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "game_pong.h"

class SDL_Clock {

public:

    SDL_Clock() = default;

    void Init()
    {
        m_tickCount = SDL_GetTicks();
    }

    float GetDeltaTime()
    {
        Uint64 CurrentTick = SDL_GetTicks();
        float DeltaTime = (CurrentTick - m_tickCount) / 1000.0f;
        m_tickCount = CurrentTick;

        // Clamp to 100ms max to prevent massive simulation jumps during lag spikes
        if (DeltaTime > 0.1f) {
            DeltaTime = 0.1f;
        }

        return DeltaTime;
    }

private:
    Uint64 m_tickCount = 0;
};

// Input State Struct to track paddle keys cleanly inside SDL3 Callbacks
struct InputState {
    bool PaddleLUp = false;
    bool PaddleLDown = false;
    bool PaddleRUp = false;
    bool PaddleRDown = false;
};

static SDL_Window* pWindow = NULL;
static SDL_Renderer* pRenderer = NULL;
static SDL_Clock GameClock;
static GameConfig Config;
static Pong Game;
static InputState Input;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Pong", "1.0", "www.ogldev.org");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL initialized");

    if (!SDL_CreateWindowAndRenderer("Pong", (int)(Config.WindowSize.x), (int)(Config.WindowSize.y),
                                     SDL_WINDOW_RESIZABLE, &pWindow, &pRenderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(pRenderer, (int)(Config.WindowSize.x),
                                    (int)(Config.WindowSize.y), SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_SetRenderVSync(pRenderer, 1);
    GameClock.Init();
    Game.Init(Config);

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event occurs (Safely handles Input Flags) */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.scancode) {
        case SDL_SCANCODE_ESCAPE:
            SDL_Log("Escape key pressed, quitting");
            return SDL_APP_SUCCESS;
        case SDL_SCANCODE_W: Input.PaddleLUp = true; break;
        case SDL_SCANCODE_S: Input.PaddleLDown = true; break;
        case SDL_SCANCODE_O: Input.PaddleRUp = true; break;
        case SDL_SCANCODE_L: Input.PaddleRDown = true; break;
        default: break;
        }
    } else if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.scancode) {
        case SDL_SCANCODE_W: Input.PaddleLUp = false; break;
        case SDL_SCANCODE_S: Input.PaddleLDown = false; break;
        case SDL_SCANCODE_O: Input.PaddleRUp = false; break;
        case SDL_SCANCODE_L: Input.PaddleRDown = false; break;
        default: break;
        }
    }

    return SDL_APP_CONTINUE;
}

static void RenderGame()
{
    // Draw background color
    SDL_SetRenderDrawColor(pRenderer, 16, 16, 16, 255);
    SDL_RenderClear(pRenderer);

    // Set entity drawing color to white
    SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);

    Rect PaddleLRect, PaddleRRect, BallRect;
    Game.GetRects(BallRect, PaddleLRect, PaddleRRect);

    SDL_FRect PaddleRect { PaddleLRect.x, PaddleLRect.y, PaddleLRect.w, PaddleLRect.h };
    SDL_RenderFillRect(pRenderer, &PaddleRect);

    PaddleRect = { PaddleRRect.x, PaddleRRect.y, PaddleRRect.w, PaddleRRect.h };
    SDL_RenderFillRect(pRenderer, &PaddleRect);

    SDL_FRect BallRectF { BallRect.x, BallRect.y, BallRect.w, BallRect.h };
    SDL_RenderFillRect(pRenderer, &BallRectF);
    SDL_RenderPresent(pRenderer);
}

/* This function runs once per frame. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    float DeltaTime = GameClock.GetDeltaTime();

    // Game updates utilizing the Event-Driven clean input state flags
    Game.Update(Input.PaddleLUp, Input.PaddleLDown, Input.PaddleRUp, Input.PaddleRDown, DeltaTime);

    RenderGame();

    return SDL_APP_CONTINUE;
}

/* Clean shutdown tracking */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 handles automatic structural destruction for window/renderer loops cleanly
    SDL_Log("Application terminated successfully.");
}

