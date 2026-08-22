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

float PaddleSpeed = 600.0f; // Pixels per second
float PaddleWidth = 30.0f;
float HalfPaddleWidth = PaddleWidth / 2.0f;
float PaddleHeight = 300.0f;
float HalfPaddleHeight = PaddleHeight / 2.0f;

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


class Paddle {

public:

    Paddle(const Vec2& pos) : m_pos(pos) {}

    void HandleUpKey(float deltaTime)
    {
        float CurrentPaddleVelocity = 0.0f;

        CurrentPaddleVelocity += PaddleSpeed;

        m_pos.y -= CurrentPaddleVelocity * deltaTime;

        if (m_pos.y - HalfPaddleHeight < 0.0f) {
            m_pos.y = HalfPaddleHeight;
        }
    }

    void HandleDownKey(float deltaTime)
    {
        float CurrentPaddleVelocity = 0.0f;

        CurrentPaddleVelocity += PaddleSpeed;

        m_pos.y += CurrentPaddleVelocity * deltaTime;

        if (m_pos.y + HalfPaddleHeight > WINDOW_HEIGHT) {
            m_pos.y = WINDOW_HEIGHT - HalfPaddleHeight;
        }
    }


    const Vec2& GetPosition() const
    {
        return m_pos;
    }

private:

    Vec2 m_pos;
};


class Ball {

public:

    Ball(const Vec2& pos, const Vec2& velocity) : m_pos(pos), m_velocity(velocity) {}

    void Update(float deltaTime)
    {
        m_pos.x += m_velocity.x * deltaTime;
        m_pos.y += m_velocity.y * deltaTime;
    }
    const Vec2& GetPosition() const
    {
        return m_pos;
    }
    const Vec2& GetVelocity() const
    {
        return m_velocity;
    }
    void SetVelocity(const Vec2& velocity)
    {
        m_velocity = velocity;
    }

    void SetPosition(const Vec2& pos)
    {
        m_pos = pos;
    }

private:

    Vec2 m_pos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
    Vec2 m_velocity = { -200.0f, 235.0f };
};


GameClock Clock;
float BallSize = 20.0f;
float HalfBallSize = BallSize / 2.0f;
Ball GameBall({ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f }, { -200.0f, 235.0f });
Paddle PaddleL({ PADDLE_OFFSET, WINDOW_HEIGHT / 2.0f });
Paddle PaddleR({ WINDOW_WIDTH - PADDLE_OFFSET, WINDOW_HEIGHT / 2.0f });
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

    float CurrentPaddleVelocity = 0.0f;

    if (pKeys[SDL_SCANCODE_W]) {
        PaddleL.HandleUpKey(DeltaTime);
    } 

    if (pKeys[SDL_SCANCODE_S]) {
        PaddleL.HandleDownKey(DeltaTime);
    }

    GameBall.Update(DeltaTime);

    // 7. Ball Ceiling / Floor Boundaries Collisions
    bool BallHitsBottom = GameBall.GetPosition().y + HalfBallSize >= WINDOW_HEIGHT;
    bool BallHitsTop = GameBall.GetPosition().y - HalfBallSize <= 0.0f;
    if ((BallHitsTop && (GameBall.GetVelocity().y < 0.0f)) || 
        (BallHitsBottom && (GameBall.GetVelocity().y > 0.0f))) {
        Vec2 NewVelocity = GameBall.GetVelocity();
        NewVelocity.y = -NewVelocity.y;
        GameBall.SetVelocity(NewVelocity);
    }

    // 8. Ball vs Paddle Precise Rect Collision Detection
    bool CollideWithPaddle =
        (GameBall.GetPosition().x - HalfBallSize <= PaddleL.GetPosition().x + HalfPaddleWidth) &&
        (GameBall.GetPosition().x + HalfBallSize >= PaddleL.GetPosition().x - HalfPaddleWidth) &&
        (GameBall.GetPosition().y + HalfBallSize >= PaddleL.GetPosition().y - HalfPaddleHeight) &&
        (GameBall.GetPosition().y - HalfBallSize <= PaddleL.GetPosition().y + HalfPaddleHeight);

    if (CollideWithPaddle && GameBall.GetVelocity().x < 0.0f) {
        Vec2 NewVelocity = GameBall.GetVelocity();
        NewVelocity.x = -NewVelocity.x;
        // Optional: Slightly boost speed upon impact to increase difficulty
        NewVelocity.x *= 1.05f;
        NewVelocity.y *= 1.05f;
        GameBall.SetVelocity(NewVelocity);
    }

    // 9. Reset Ball if it goes out of bounds (Left Wall Point Loss)
    if (GameBall.GetPosition().x < 0.0f) {
        GameBall.SetPosition({ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f });
        GameBall.SetVelocity({ -200.0f, 235.0f }); // Reset speed
    }

    // Bounce off right wall for now since AI/Right paddle isn't written yet
    if (GameBall.GetPosition().x + HalfBallSize >= WINDOW_WIDTH && GameBall.GetVelocity().x > 0.0f) {
        Vec2 NewVelocity = GameBall.GetVelocity();
        NewVelocity.x = -NewVelocity.x;
        GameBall.SetVelocity(NewVelocity);
    }

    // 10. Render Geometries to screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_FRect PaddleRect{ PaddleL.GetPosition().x - HalfPaddleWidth, 
                          PaddleL.GetPosition().y - HalfPaddleHeight, 
                          PaddleWidth, PaddleHeight };

    SDL_RenderFillRect(renderer, &PaddleRect);

    SDL_FRect BallRect{ GameBall.GetPosition().x - HalfBallSize, GameBall.GetPosition().y - HalfBallSize, BallSize, BallSize };
    SDL_RenderFillRect(renderer, &BallRect);
    SDL_RenderPresent(renderer);    
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

