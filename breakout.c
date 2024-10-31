#include "raylib.h"

#define PLAYERS_LIFE        5
#define BRICK_LINES         5
#define BRICKS_PER_LINE     20
#define BRICKS_POSITION_y   50

typedef enum GameScreen{LOGO, TITLE, GAMEPLAY, ENDING} GameScreen;

typedef struct Player
{
    Vector2 position;
    Vector2 speed;
    Vector2 size;
    Rectangle bounds;
    int lives;
}Player;

typedef struct Ball
{
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
}Ball;

typedef struct Brick
{
    Vector2 position;
    Vector2 size;
    Rectangle bounds;
    int resistance;
    bool active;
}Brick;

bool allBricksBroken(Brick bricks[BRICK_LINES][BRICKS_PER_LINE])
{
    for(int i = 0; i < BRICK_LINES; i++)
    {
        for(int j = 0; j < BRICKS_PER_LINE; j++)
        {
            if(bricks[i][j].active)return false;
        }
    }
    return true;
}

int main()
{
    const int screenHeight = 450;
    const int screenWidth  = 800;

    InitWindow(screenWidth, screenHeight, "PROJECT: BLOCKS GAME");

    Texture2D texLogo = LoadTexture("resources/raylib_logo.png");
    Texture2D texBall = LoadTexture("resources/ball.png");
    Texture2D texPaddle = LoadTexture("resources/paddle.png");
    Texture2D texBrick = LoadTexture("resources/brick.png");

    Font font = LoadFont("resources/setback.png");

    InitAudioDevice();

    Sound fxStart = LoadSound("resources/game_bonus.mp3");
    Sound fxBounce = LoadSound("resources/balloon_bounce.wav");
    Sound fxExplode = LoadSound("resources/impact.mp3");
    Sound fxGameover = LoadSound("resources/game_over.mp3");
    Sound fxGameWon = LoadSound("resources/game_won.mp3");

    Music music = LoadMusicStream("resources/arcade.mp3");

    PlayMusicStream(music);

    GameScreen screen = LOGO;

    int framesCounter = 0;
    int gameResult = -1;
    bool gamePaused = false;

//initialising struct variable
    Player player = {0};
    Ball ball = {0};
    Brick bricks [BRICK_LINES][BRICKS_PER_LINE] = {0};

//initialising player struct
    player.position = (Vector2){screenWidth/2.0, screenHeight*7.0/8.0};
    player.speed = (Vector2){8.0f, 0.0f};
    player.size = (Vector2){100, 24};
    player.lives = PLAYERS_LIFE;

//initialising ball struct
    ball.radius = 10.0f;
    ball.active = false;
    ball.position = (Vector2){player.position.x + player.size.x/2, player.position.y - ball.radius};
    ball.speed = (Vector2){4.0f, 4.0f};

//brick position logic
    for(int i=0; i<BRICK_LINES; i++ )
    {
        for(int j=0; j<BRICKS_PER_LINE; j++)
        {

            bricks[i][j].size = (Vector2){(float)screenWidth/BRICKS_PER_LINE, 20};
            bricks[i][j].position = (Vector2){j*bricks[i][j].size.x, i*bricks[i][j].size.y + BRICKS_POSITION_y};

            bricks[i][j].bounds = (Rectangle){bricks[i][j].position.x, bricks[i][j].position.y, bricks[i][j].size.x, bricks[i][j].size.y};
            bricks[i][j].active = true;
        }
    }

    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        switch(screen) //game logic handling
        {
            case LOGO:
            {
                framesCounter++;
                if (framesCounter>180)
                {
                    screen = TITLE;
                    framesCounter = 0;
                }
            }break;

            case TITLE:
            {
                framesCounter++;
                if(IsKeyPressed(KEY_ENTER))
                {
                    screen = GAMEPLAY;
                    PlaySound(fxStart);
                }
                
            }break;

            case GAMEPLAY:
            {
                if(IsKeyPressed('P'))gamePaused = !gamePaused;

                if(!gamePaused)
                {
                    //player movement logic
                    if(IsKeyDown(KEY_LEFT))player.position.x -= player.speed.x;
                    if(IsKeyDown(KEY_RIGHT))player.position.x += player.speed.x;

                    if(player.position.x <= 0)player.position.x = 0;
                    if((player.position.x + player.size.x) >= screenWidth) player.position.x = screenWidth - player.size.x;

                    player.bounds = (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y};

                    if(ball.active)
                    {
                        //ball movement logic
                        ball.position.x += ball.speed.x;
                        ball.position.y += ball.speed.y;

                        //ball vs screen limits collision logic
                        if((ball.position.x + ball.radius) >= screenWidth || (ball.position.x - ball.radius) <= 0) ball.speed.x *= -1;
                        if((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;

                        //ball vs player bar collision logic
                        if(CheckCollisionCircleRec(ball.position, ball.radius, player.bounds))
                        {
                            ball.speed.y *= -1;
                            ball.speed.x = (ball.position.x - (player.position.x + player.size.x/2))/player.size.x*5.0f;
                            PlaySound(fxBounce);
                        }

                        //ball vs bricks collision logic
                        for(int i = 0; i < BRICK_LINES; i++)
                        {
                            for(int j = 0; j < BRICKS_PER_LINE; j++)
                            {
                                if(bricks[i][j].active && CheckCollisionCircleRec(ball.position, ball.radius, bricks[i][j].bounds))
                                {
                                    bricks[i][j].active = false;
                                    ball.speed.y *= -1;
                                    PlaySound(fxExplode);

                                    break;
                                }
                            }
                        }

                        //game ending logic

                        //condition when ball wasnt received by the player
                        if((ball.position.y + ball.radius) >= screenHeight)
                        {
                            ball.position.x = player.position.x + player.size.x/2;
                            ball.position.y = player.position.y - ball.radius - 1.0f;
                            ball.speed = (Vector2){0.0f, 0.0f};
                            ball.active = false;

                            player.lives--;
                        }

                        //Restart game
                        if(player.lives < 0)
                        {
                            screen = ENDING;
                            PlaySound(fxGameover);

                            player.lives = 5;
                            framesCounter = 0;
                        }

                        if(allBricksBroken(bricks))
                        {
                            screen = ENDING;
                            PlaySound(fxGameWon);
                            gameResult = 1;
                            framesCounter = 0;
                        }
                    }

                    else{
                        ball.position.x = player.position.x + player.size.x/2;

                        if(IsKeyPressed(KEY_SPACE))
                        {
                            ball.active = true;
                            ball.speed = (Vector2){0, -5.0f};
                        }
                    }
                }
            }break;

            case ENDING:
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    screen = TITLE;
                    gameResult = -1;  // Reset the game result
                    player.lives = PLAYERS_LIFE;  // Reset player lives

                    // Reset ball position and status
                    ball.active = false;
                    ball.position = (Vector2){player.position.x + player.size.x/2, player.position.y - ball.radius};

                    // Reset brick states
                    for(int i = 0; i < BRICK_LINES; i++)
                    {
                        for(int j = 0; j < BRICKS_PER_LINE; j++)
                        {
                            bricks[i][j].active = true;
                        }
                    }
                    framesCounter = 0;  // Reset frame counter
                }
              }break;
            default:break;
        }

        UpdateMusicStream(music);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        switch(screen)//game rendering part
        {
            case LOGO:
            {
                DrawTexture(texLogo, screenWidth/2 - texLogo.width/2, screenHeight/2 - texLogo.height/2, WHITE);
            }break;

            case TITLE:
            {

                DrawTextEx(font, "BLOCKS", (Vector2){100,80}, 160, 10, MAROON);//Drawing the title "Blocks"

                if((framesCounter/30)%2==0)
                {
                    DrawText("PRESS [ENTER] TO START", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO START",20)/2, GetScreenHeight()/2+60, 20, DARKGRAY);
                }
            }break;

            case GAMEPLAY:
            {

                #define LESSON05_TEXTURES
                #if defined(LESSON02_SHAPES)

                //drawing the player bar
                DrawRectangle(player.position.x, player.position.y, player.size.x, player.size.y, BLACK);

                //drawing the ball
                DrawCircleV(ball.position, ball.radius, MAROON); 
                
                //drawing the bricks
                for(int i=0; i<BRICK_LINES; i++)
                {
                    for(int j=0; j<BRICKS_PER_LINE; j++)
                    {
                        if(bricks[i][j].active)
                        {
                            if((i+j)%2==0)
                            DrawRectangle(bricks[i][j].position.x, bricks[i][j].position.y, bricks[i][j].size.x, bricks[i][j].size.y, GRAY);
                            else
                            DrawRectangle(bricks[i][j].position.x, bricks[i][j].position.y, bricks[i][j].size.x, bricks[i][j].size.y, DARKGRAY);
                        }
                    }
                }
                #elif defined (LESSON05_TEXTURES)

                DrawTextureEx(texPaddle, player.position, 0.0f, 1.0f, WHITE);// drawing paddle using texture
                DrawTexture(texBall, ball.position.x - ball.radius/2, ball.position.y - ball.radius/2, MAROON);//drawing ball using texture

                //drawing bricks
                for(int i = 0; i < BRICK_LINES; i++)
                {
                    for(int j = 0; j < BRICKS_PER_LINE; j++)
                    {
                        if(bricks[i][j].active)
                        {
                            if((i+j)%2==0)DrawTextureEx(texBrick, bricks[i][j].position, 0.0f, 1.0f, GRAY);
                            else DrawTextureEx(texBrick, bricks[i][j].position, 0.0f, 1.0f, DARKGRAY);
                        }
                    }
                }
                #endif

                //drawing player lives indication
                for(int i=0; i<player.lives; i++)
                    DrawRectangle(20 + i*40, screenHeight-30, 35, 10, LIGHTGRAY);
                
                if(gamePaused)
                DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 + 60, 40, GRAY);
            }break;

            case ENDING:
            {

                if (gameResult == 1){
                DrawTextEx(font, "YOU WIN!!", (Vector2){200, 100}, 80, 6, MAROON);
                }
                else {
                DrawTextEx(font, "GAME OVER", (Vector2){200, 100}, 80, 6, MAROON);
                }

                if ((framesCounter / 30) % 2 == 0){
                DrawText("PRESS [ENTER] TO PlAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 + 80, 20, GRAY);
                }
            } break;
            default:break;
        }
        EndDrawing();
    }
    
    UnloadTexture(texBall);
    UnloadTexture(texLogo);
    UnloadTexture(texPaddle);
    UnloadTexture(texBrick);

    UnloadMusicStream(music);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}


