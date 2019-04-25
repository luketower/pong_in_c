#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 480
#define PADDLE_DISTANCE_FROM_WALL 50
#define FALSE 0
#define TRUE 1
#define bool u8
#define GREEN 0x21fb00
#define global static

typedef uint32_t u32;
typedef uint8_t u8;;

enum keyboard_press {
    PRESSED_UNDEFINED,
    PRESSED_UP,
    PRESSED_DOWN,
    PRESSED_SPACE
};

enum game_state {
    GAME_READY,
    GAME_PLAY,
    GAME_OVER
};

global game_state State;

global int ScoreNumbers[4][15] =
{
    {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        0, 1, 1,
        0, 0, 1,
        1, 1, 1,
    }
};

struct position {
    float X;
    float Y;
};

struct paddle {
    position Position;
    float Width;
    float Height;
    float Speed;
    int Score;
};

struct ball {
    position Position;
    float VelocityX;
    float VelocityY;
    float Radius;
};

global int SCREEN_VERTICAL_CENTER = SCREEN_HEIGHT/2;
global int SCREEN_HORIZONTAL_CENTER = SCREEN_WIDTH/2;
global position SCREEN_CENTER = { SCREEN_HORIZONTAL_CENTER, SCREEN_VERTICAL_CENTER };

float Lerp(float Start, float End, float Percent)
{
    return Start + (End-Start)*Percent;
}

void ResetPaddlePositions(paddle *LeftPaddle, paddle *RightPaddle)
{
    LeftPaddle->Position.Y = SCREEN_VERTICAL_CENTER;
    RightPaddle->Position.Y = SCREEN_VERTICAL_CENTER;
}

void UpdateBall(ball *Ball, paddle *LeftPaddle, paddle *RightPaddle, float ElapsedTime)
{
    Ball->Position.X += Ball->VelocityX * (ElapsedTime/1000);
    Ball->Position.Y += Ball->VelocityY * (ElapsedTime/1000);

    if((Ball->Position.Y < 0) ||
       (Ball->Position.Y > SCREEN_HEIGHT))
    {
        Ball->VelocityY = -Ball->VelocityY;
        return;
    }

    if(Ball->Position.X < 0)
    {
        RightPaddle->Score++;
        ResetPaddlePositions(LeftPaddle, RightPaddle);
        Ball->Position = SCREEN_CENTER;
        State = GAME_READY;
        return;
    }
    else if(Ball->Position.X > SCREEN_WIDTH)
    {
        LeftPaddle->Score++;
        ResetPaddlePositions(LeftPaddle, RightPaddle);
        Ball->Position = SCREEN_CENTER;
        State = GAME_READY;
        return;
    }

    if(Ball->Position.X < (LeftPaddle->Position.X + LeftPaddle->Width/2))
    {
        if((Ball->Position.Y > (LeftPaddle->Position.Y - LeftPaddle->Height/2)) &&
           (Ball->Position.Y < (LeftPaddle->Position.Y + LeftPaddle->Height/2)))
        {
            Ball->VelocityX = -Ball->VelocityX;
            return;
        }
    }

    if(Ball->Position.X > (RightPaddle->Position.X - RightPaddle->Width/2))
    {
        if((Ball->Position.Y > (RightPaddle->Position.Y - RightPaddle->Height/2)) &&
           (Ball->Position.Y < (RightPaddle->Position.Y + RightPaddle->Height/2)))
        {
            Ball->VelocityX = -Ball->VelocityX;
        }
    }
}

void UpdatePaddle(paddle *Paddle, keyboard_press Pressed, float ElapsedTime)
{
    if(Pressed == PRESSED_UP && ((Paddle->Position.Y - Paddle->Height/2) > 0))
    {
        Paddle->Position.Y -= Paddle->Speed * (ElapsedTime/1000);
    }

    if((Pressed == PRESSED_DOWN) &&
        ((Paddle->Position.Y + Paddle->Height/2) < SCREEN_HEIGHT))
    {
        Paddle->Position.Y += Paddle->Speed * (ElapsedTime/1000);
    }
}

void UpdateAiPaddle(paddle *Paddle, ball Ball)
{
    Paddle->Position.Y = Ball.Position.Y;
}

void DrawScore(position Position, int Size, int Num, u32 *ScreenPixels)
{
    int StartX = Position.X - (Size*3)/2;
    int StartY = Position.Y - (Size*5)/2;

    for(int i = 0; i < 15; i++)
    {
        if(ScoreNumbers[Num][i] == 1)
        {
            for(int y = StartY; y < StartY+Size; y++)
            {
                for(int x = StartX; x < StartX+Size; x++)
                {
                    ScreenPixels[(y)*SCREEN_WIDTH + x] = GREEN;
                }
            }
        }
        StartX += Size;
        if((i+1)%3 == 0)
        {
            StartY += Size;
            StartX -= Size * 3;
        }
    }
}

ball InitBall()
{
    ball Ball;

    Ball.Position.X = SCREEN_HORIZONTAL_CENTER;
    Ball.Position.Y = SCREEN_VERTICAL_CENTER;
    Ball.VelocityX = 150;
    Ball.VelocityY = 150;
    Ball.Radius = 30;

    return Ball;
}

paddle InitPaddle(bool IsLeftPaddle)
{
    paddle Paddle;

    if(IsLeftPaddle)
    {
        Paddle.Position.X = PADDLE_DISTANCE_FROM_WALL;
    }
    else
    {
        Paddle.Position.X = SCREEN_WIDTH - PADDLE_DISTANCE_FROM_WALL;
    }

    Paddle.Position.Y = SCREEN_VERTICAL_CENTER;
    Paddle.Width = 5;
    Paddle.Height = 40;
    Paddle.Speed = 600;
    Paddle.Score = 0;

    return Paddle;
}

void DrawPaddle(paddle Paddle, u32 *ScreenPixels)
{
    SDL_assert(ScreenPixels);

    int StartX = Paddle.Position.X - Paddle.Width/2;
    int StartY = Paddle.Position.Y - Paddle.Height/2;

    for(int Row = 0; Row < Paddle.Height; Row++)
    {
        for(int Col = 0; Col < Paddle.Width; Col++)
        {
            ScreenPixels[(Row+StartY)*SCREEN_WIDTH + Col + StartX] = GREEN;
        }
    }

    int ScoreX = Lerp(Paddle.Position.X, SCREEN_CENTER.X, 0.2);
    position ScorePosition = { ScoreX, 35 };
    DrawScore(ScorePosition, 5, Paddle.Score, ScreenPixels);
}

void DrawBall(ball Ball, u32 *ScreenPixels)
{
    SDL_assert(ScreenPixels);

    for(int Row = -Ball.Radius; Row < Ball.Radius; Row++)
    {
        for(int Col = -Ball.Radius; Col < Ball.Radius; Col++)
        {
            if(Row*Row+Col*Col < Ball.Radius)
            {
                ScreenPixels[(int)(Row+Ball.Position.Y)*SCREEN_WIDTH + Col +(int)Ball.Position.X] = GREEN;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *Window = SDL_CreateWindow("Pong Song",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    SDL_assert(Window);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, 0, SDL_RENDERER_SOFTWARE);
    SDL_assert(Renderer);

    SDL_Texture *Screen = SDL_CreateTexture(Renderer,
                                            SDL_PIXELFORMAT_RGB888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            SCREEN_WIDTH,
                                            SCREEN_HEIGHT);
    SDL_assert(Screen);

    u32 *ScreenPixels = (u32*) calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(u32));
    SDL_assert(ScreenPixels);

    bool Done = FALSE;
    keyboard_press Pressed = PRESSED_UNDEFINED;

    ball Ball = InitBall();

    bool IsLeftPaddle = TRUE;
    paddle LeftPaddle = InitPaddle(IsLeftPaddle);

    IsLeftPaddle = FALSE;
    paddle RightPaddle = InitPaddle(IsLeftPaddle);

    u32 FrameStart;
    float ElapsedTime;
    State = GAME_READY;

    while (!Done)
    {
        FrameStart = SDL_GetTicks();
        SDL_Event Event;

        while (SDL_PollEvent(&Event))
        {
            SDL_Keycode KeyCode = Event.key.keysym.sym;

            switch (KeyCode)
            {
                case SDLK_ESCAPE:
                case SDLK_q:
                    Done = TRUE;
                    break;
                case SDLK_UP:
                    Pressed = PRESSED_UP;
                    break;
                case SDLK_DOWN:
                    Pressed = PRESSED_DOWN;
                    break;
                case SDLK_SPACE:
                    Pressed = PRESSED_SPACE;
                default:
                    break;
            }
        }


        if(State == GAME_PLAY)
        {
            UpdatePaddle(&LeftPaddle, Pressed, ElapsedTime);
            UpdateAiPaddle(&RightPaddle, Ball); // Just tracking the ball for now.
            UpdateBall(&Ball, &LeftPaddle, &RightPaddle, ElapsedTime);
        }
        else if(State == GAME_READY)
        {
            if(Pressed == PRESSED_SPACE)
            {
                State = GAME_PLAY;

                if (LeftPaddle.Score == 3 || RightPaddle.Score == 3)
                {
                    LeftPaddle.Score = 0;
                    RightPaddle.Score = 0;
                }
            }
        }

        memset(ScreenPixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));

        DrawPaddle(LeftPaddle, ScreenPixels);
        DrawPaddle(RightPaddle, ScreenPixels);
        DrawBall(Ball, ScreenPixels);

        SDL_UpdateTexture(Screen, NULL, ScreenPixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer, Screen, NULL, NULL);
        SDL_RenderPresent(Renderer);

        Pressed = PRESSED_UNDEFINED;
        ElapsedTime = (float)(SDL_GetTicks() - FrameStart);

        if(ElapsedTime < 6)
        {
            SDL_Delay(6 - (u32)(ElapsedTime));
            ElapsedTime = (float)(SDL_GetTicks() - FrameStart);
        }
    }

    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}
