#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "pong.h"

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


class SDLPong {

public:

    SDLPong() = default;

    bool Init()
    {
        if (!SDL_CreateWindowAndRenderer("Pong", (int)(m_config.WindowSize.x), (int)(m_config.WindowSize.y),
            SDL_WINDOW_RESIZABLE, &m_pWindow, &m_pRenderer)) {
            SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
            return false;
        }

        SDL_SetRenderLogicalPresentation(m_pRenderer, (int)(m_config.WindowSize.x),
            (int)(m_config.WindowSize.y), SDL_LOGICAL_PRESENTATION_LETTERBOX);

        SDL_SetRenderVSync(m_pRenderer, 1);
        m_gameClock.Init();
        m_game.Init(m_config);

        return true;
    }


    SDL_AppResult HandleInput(const SDL_Event* pEvent)
    {
        if (pEvent->type == SDL_EVENT_QUIT) {
            return SDL_APP_SUCCESS;
        }

        if (pEvent->type == SDL_EVENT_KEY_DOWN) {
            switch (pEvent->key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                SDL_Log("Escape key pressed, quitting");
                return SDL_APP_SUCCESS;
            case SDL_SCANCODE_R: m_input.PaddleLUp = true; break;
            case SDL_SCANCODE_F: m_input.PaddleLDown = true; break;
            case SDL_SCANCODE_O: m_input.PaddleRUp = true; break;
            case SDL_SCANCODE_L: m_input.PaddleRDown = true; break;
            default: break;
            }
        } else if (pEvent->type == SDL_EVENT_KEY_UP) {
            switch (pEvent->key.scancode) {
            case SDL_SCANCODE_R: m_input.PaddleLUp = false; break;
            case SDL_SCANCODE_F: m_input.PaddleLDown = false; break;
            case SDL_SCANCODE_O: m_input.PaddleRUp = false; break;
            case SDL_SCANCODE_L: m_input.PaddleRDown = false; break;
            default: break;
            }
        }

        return SDL_APP_CONTINUE;
    }


    void UpdateAndRender()
    {
        float DeltaTime = m_gameClock.GetDeltaTime();

        // Game updates utilizing the Event-Driven clean input state flags
        m_game.Update(DeltaTime, m_input.PaddleLUp, m_input.PaddleLDown, m_input.PaddleRUp, m_input.PaddleRDown);

        RenderFrame();
    }

private:

    void RenderFrame()
    {
        // Draw background color
        SDL_SetRenderDrawColor(m_pRenderer, 16, 16, 16, 255);
        SDL_RenderClear(m_pRenderer);

        Rect PaddleLRect, PaddleRRect, BallRect;
        m_game.GetRects(BallRect, PaddleLRect, PaddleRRect);

        SDL_FRect PaddleRect{ PaddleLRect.x, PaddleLRect.y, PaddleLRect.w, PaddleLRect.h };
        SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 255, 255);
        SDL_RenderFillRect(m_pRenderer, &PaddleRect);

        PaddleRect = { PaddleRRect.x, PaddleRRect.y, PaddleRRect.w, PaddleRRect.h };
        SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
        SDL_RenderFillRect(m_pRenderer, &PaddleRect);

        SDL_FRect BallRectF{ BallRect.x, BallRect.y, BallRect.w, BallRect.h };
        SDL_SetRenderDrawColor(m_pRenderer, 255, 255, 0, 255);
        SDL_RenderFillRect(m_pRenderer, &BallRectF);

        SDL_RenderPresent(m_pRenderer);
    }

    SDL_Window* m_pWindow = NULL;
    SDL_Renderer* m_pRenderer = NULL;
    SDL_Clock m_gameClock;
    GameConfig m_config;
    Pong m_game;
    PongInputState m_input;
};


SDLPong g_sdlPong;


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Pong", "1.0", "www.ogldev.org");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!g_sdlPong.Init()) {
        SDL_Log("Failed to initialize SDL Pong");
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL initialized");

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event occurs (Safely handles Input Flags) */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return g_sdlPong.HandleInput(event);
}


/* This function runs once per frame. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    g_sdlPong.UpdateAndRender();
    return SDL_APP_CONTINUE;
}

/* Clean shutdown tracking */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 handles automatic structural destruction for window/renderer loops cleanly
    SDL_Log("Application terminated successfully.");
}

