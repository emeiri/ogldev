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
#include "game_pong.h"

class SDL_Clock {

public:

    SDL_Clock() {}

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


/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Clock Clock;
static GameConfig Config;
static Pong Game;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Pong", "1.0", "com.example.CATEGORY-NAME");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL initialized");

    if (!SDL_CreateWindowAndRenderer("Pong", (int)Config.WindowSize.x, (int)Config.WindowSize.y, 
                                     SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(renderer, (int)Config.WindowSize.x, 
                                     (int)Config.WindowSize.y, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_SetRenderVSync(renderer, 1);

    Clock.Init();

    Game.Init(Config);

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


static void RenderGame()
{
    SDL_SetRenderDrawColor(renderer, 16, 16, 16, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    Rect PaddleLRect, PaddleRRect, BallRect;
    Game.GetRects(BallRect, PaddleLRect, PaddleRRect);

    SDL_FRect PaddleRect { PaddleLRect.x, PaddleLRect.y, PaddleLRect.w, PaddleLRect.h };
    SDL_RenderFillRect(renderer, &PaddleRect);

    SDL_FRect BallRectF { BallRect.x, BallRect.y, BallRect.w, BallRect.h };
    SDL_RenderFillRect(renderer, &BallRectF);
    SDL_RenderPresent(renderer);
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    float DeltaTime = Clock.GetDeltaTime();

   // printf("DeltaTime: %f\n", DeltaTime);
    int NumKeys = 0;
    const bool* pKeys = SDL_GetKeyboardState(&NumKeys);

    bool PaddleLUp = pKeys[SDL_SCANCODE_W];
    bool PaddleLDown = pKeys[SDL_SCANCODE_S];
    bool PaddleRUp = pKeys[SDL_SCANCODE_O];
    bool PaddleRDown = pKeys[SDL_SCANCODE_P];

    Game.Update(PaddleLUp, PaddleLDown, PaddleRUp, PaddleRDown, DeltaTime);    

    RenderGame();

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

