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

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

int PaddleDirection = 0; // -1 for up, 1 for down, 0 for no movement
Uint64 TickCount = 0;


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

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    int NumKeys = 0;
    const bool* pKeys = SDL_GetKeyboardState(&NumKeys);

    if (pKeys[SDL_SCANCODE_ESCAPE]) {
        SDL_Log("Escape key pressed, quitting");
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (pKeys[SDL_SCANCODE_W]) {
        PaddleDirection -= 4; // Move paddle up
        PaddleDirection = std::max(PaddleDirection, -20); // Clamp to -20
    } 

    if (pKeys[SDL_SCANCODE_S]) {
        PaddleDirection += 4; // Move paddle down
        PaddleDirection = std::min(PaddleDirection, 20); // Clamp to 20
    } 

  //  printf("PaddleDirection: %d\n", PaddleDirection);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

float BallSize = 20.0f;
Vec2 BallPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
Vec2 PaddlePos = { 25.0f, WINDOW_HEIGHT / 2.0f };
float PaddleWidth = 30.0f;
float PaddleHeight = 300.0f;

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    Uint64 NewTickCount = SDL_GetTicks();
    float DeltaTime = (NewTickCount - TickCount) / 1000.0f;
    TickCount = NewTickCount;
   // printf("DeltaTime: %f\n", DeltaTime);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    if (PaddleDirection != 0) {
        PaddlePos.y += PaddleDirection * 50.0f * DeltaTime; // Move paddle at 500 pixels per second

        if (PaddleDirection > 0) {
            PaddleDirection -= 1; // Gradually reduce the direction to 0
        } else {
            PaddleDirection += 1; // Gradually reduce the direction to 0
        }
    }

    SDL_FRect Paddle(PaddlePos.x - PaddleWidth / 2.0f, PaddlePos.y - PaddleHeight / 2.0f, PaddleWidth, PaddleHeight);
    SDL_RenderFillRect(renderer, &Paddle);

    SDL_FRect Ball(BallPos.x - BallSize / 2.0f, BallPos.y - BallSize / 2.0f, BallSize, BallSize);    
    SDL_RenderFillRect(renderer, &Ball);

    SDL_RenderPresent(renderer);    
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

