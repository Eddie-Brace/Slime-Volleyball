/* 
 * File:   main.cpp
 * Author: Edward Brace
 * A desktop recreation of Quinn Pendragon's classic 'Slime Volleyball' game.
 */

#include <cstdlib>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define WINDOW_WIDTH 1350
#define SLIME_WALK_SPEED 11
#define SLIME_JUMP_SPEED 40
#define SLIME_RADIUS 75
#define BALL_RADIUS 25
#define MAX_BALL_SPEED_X 12
#define MAX_BALL_SPEED_Y 10

using namespace std;
void drawSlime( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius );
void drawBall( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius, SDL_Color edgeColor, SDL_Color fillColor );
int detectCollision( struct movingObject * slime, struct movingObject * ball );
void pointScored( SDL_Renderer * renderer, int slimeNum, TTF_Font * font );
void victoryAchieved( SDL_Renderer * renderer, int slimeNum, TTF_Font * font );
void receiveOpponentKeyState( Uint8 * oppKeys, int socket, const Uint8 * myKeys );

int slime1Score = 0;
int slime2Score = 0;

struct movingObject {  int x;   int y;    int velX;  int velY; };

/*
 * this runs the game; currently no params
 */
int main(int argc, char** argv) {
    
    /**** init. SDL, load resources, create window
     */
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 ) { //mandatory error checking
        printf( "Error initializing SDL: %s", SDL_GetError() );
        return 1;
    }
    //initialize fonts
    if( TTF_Init() != 0 ) { //mandatory error checking
        printf( "Error initializing SDL/TTF: %s", SDL_GetError() );
        return 1;
    }
    TTF_Font * font = TTF_OpenFont( "Gameplay.ttf", 50 );
    
    //create window
    SDL_Window * window = SDL_CreateWindow( "Slime Volleyball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, 650, 0 );
    if( !window ) { //mandatory error checking
    
        printf( "Error creating window: %s", SDL_GetError() );
        SDL_Quit();
        return 1;
    }

    //create graphics renderer
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer * renderer = SDL_CreateRenderer( window, -1, render_flags );
    if( !renderer ) { //mandatory error checking
    
        printf( "Error creating renderer: %s", SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }
    
    
    /**** create basic game objects
     */
    int ballSpawnX = WINDOW_WIDTH / 4;
    
    start: //this is where we return to after each point
    struct movingObject slime1;
        slime1.x = WINDOW_WIDTH / 4;
        slime1.y = 550;
        slime1.velX = 0;
        slime1.velY = 0;
    struct movingObject slime2;
        slime2.x = WINDOW_WIDTH * .75;
        slime2.y = 550;
        slime2.velX = 0;
        slime2.velY = 0;
    struct movingObject ball;
        ball.x = ballSpawnX;
        ball.y = 350;
        ball.velX = 0;
        ball.velY = -4;
    int gravity = 0;

    
    /***start running client, attempt to connect to server
    ***/
    struct addrinfo hints;
    struct addrinfo *res;
    
    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    int status = getaddrinfo( argv[1], argv[2], &hints, &res);
        
    if( status != 0 ) {
        printf( "%s", "Could not connect to server." );
        exit(1);
    }
    
    
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if( sock == -1 || connect(sock, res->ai_addr, res->ai_addrlen) == -1 ) {
        printf( "%s", "Could not connect to server." );
        exit(1);
    }
    printf( "%s\n", "successfully connected to server!" );
    
    /***
     * now that all the init work is complete, is time to animate and interact w/ user in loop until game end
     */
    while( 1 ) {
    
        //draw blue sky background and sand beach
        SDL_SetRenderDrawColor(renderer, 0, 230, 255, 255 );
        SDL_RenderClear( renderer );
    
        SDL_Rect sand;
            sand.x = 0;
            sand.y = 550;
            sand.w = WINDOW_WIDTH;
            sand.h = 100;
        SDL_SetRenderDrawColor( renderer, 250, 242, 195, 255 );
        SDL_RenderFillRect( renderer, &sand );
        
        
        //move slimes and ball to new position
        slime1.x += slime1.velX;
        slime1.y -= slime1.velY;
        slime2.x += slime2.velX;
        slime2.y -= slime2.velY;
        ball.x += ball.velX;
        ball.y -= ball.velY;
        //prevent slimes from going out of bounds
        if( slime1.y > 550 )
            slime1.y = 550;
        if( slime2.y > 550 )
            slime2.y = 550;
        
        if( slime1.x < SLIME_RADIUS )
            slime1.x = SLIME_RADIUS - 1;
        else if( slime1.x > (WINDOW_WIDTH / 2) - 8 - SLIME_RADIUS)
            slime1.x = (WINDOW_WIDTH / 2) - 8 - SLIME_RADIUS;
        
        if( slime2.x > WINDOW_WIDTH - SLIME_RADIUS )
            slime2.x = WINDOW_WIDTH - SLIME_RADIUS;
        else if( slime2.x < (WINDOW_WIDTH / 2) + 8 + SLIME_RADIUS )
            slime2.x = (WINDOW_WIDTH / 2) + 8 + SLIME_RADIUS - 1;
    
        
        //draw slimes, net, & ball
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        drawSlime( renderer, slime1.x, slime1.y, SLIME_RADIUS );
        SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
        drawSlime( renderer, slime2.x, slime2.y, SLIME_RADIUS );
    
        SDL_Rect net;
            net.x = (WINDOW_WIDTH / 2) - 8;
            net.y = 420; //DUDE
            net.w = 16;
            net.h = 130;
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderFillRect( renderer, &net );
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255 );
        SDL_RenderDrawRect( renderer, &net );
    
        SDL_Color edgeColor = { 0, 0, 0, 255 };
        SDL_Color fillColor = { 245, 245, 245, 255 };
        drawBall( renderer, ball.x, ball.y, BALL_RADIUS, edgeColor, fillColor );
        
        /**
         * draw scoreboard & check for victory conditions
         * give victory message & end game if applicable
         */
        SDL_Color fillColor1 = { 255, 0, 0, 255 };
        SDL_Color emptyFill = { 255, 255, 0, 255 };
        for( int i = 1; i <= slime1Score; i++ )
            drawBall( renderer, i * 50, 30, 20, edgeColor, fillColor1 );
        for( int j = slime1Score + 1; j <= 7; j++ )
            drawBall( renderer, j * 50, 30, 20, edgeColor, emptyFill );
        
        SDL_Color fillColor2 = { 0, 0, 255, 255 };
        for( int i = 1; i <= slime2Score; i++ )
            drawBall( renderer, WINDOW_WIDTH - i * 50, 30, 20, edgeColor, fillColor2 );
        for( int j = slime2Score + 1; j <= 7; j++ )
            drawBall( renderer, WINDOW_WIDTH - j * 50, 30, 20, edgeColor, emptyFill );
        
        int gameOver = 0;
        if( slime1Score == 7 ) {
            victoryAchieved( renderer, 1, font );
            gameOver = 1;
        }
        if( slime2Score == 7 ) {
            victoryAchieved( renderer, 2, font );
            gameOver = 1;
        }
    
    
        //print graphics to visible buffer
        SDL_RenderPresent( renderer );
    
        //close program on window exit or OS 'quit' signal
        SDL_Event e;
        SDL_PollEvent( &e );
        if( e.type == SDL_QUIT || gameOver ) {
            //clean up and end program; need to make this its own function/object at some point
            close( sock );
            freeaddrinfo( res );
            
            TTF_CloseFont( font );
            SDL_DestroyRenderer( renderer );
            SDL_DestroyWindow( window );
            SDL_Quit();
    
            return 0;
        }
        /**code for point on ball->ground impact here
         */
        if( ball.y >= 550 - BALL_RADIUS ) {
            if( ball.x <= WINDOW_WIDTH / 2 - 8 ) {
                pointScored( renderer, 2, font );
                ballSpawnX = WINDOW_WIDTH * .75;
                slime2Score++;
            }
            else {
                pointScored( renderer, 1, font );
                ballSpawnX = WINDOW_WIDTH / 4;
                slime1Score++;
            }
            goto start;
        }
        
        
        //receive opponent's keypress and store
        Uint8 * oppKeys;
        const Uint8 * keys = SDL_GetKeyboardState( NULL );
        receiveOpponentKeyState( oppKeys, sock, keys );
        //read keys to interpret players movement
        
        
        if( keys[SDL_SCANCODE_W] && slime2.y == 550)
            slime2.velY = SLIME_JUMP_SPEED;
        if( keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D] )
            slime2.velX = -SLIME_WALK_SPEED;
        else if( keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime2.velX = SLIME_WALK_SPEED;
        else if( !keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime2.velX = 0;
            
        if( oppKeys[SDL_SCANCODE_W] && slime1.y == 550)
            slime1.velY = SLIME_JUMP_SPEED;
        if( oppKeys[SDL_SCANCODE_A] && !oppKeys[SDL_SCANCODE_D] )
            slime1.velX = -SLIME_WALK_SPEED;
        else if( oppKeys[SDL_SCANCODE_D] && !oppKeys[SDL_SCANCODE_A] )
            slime1.velX = SLIME_WALK_SPEED;
        else if( !oppKeys[SDL_SCANCODE_D] && !oppKeys[SDL_SCANCODE_A] )
            slime1.velX = 0;
        
        
        //take gravity and ground force into account for slimes
        if( slime1.velY >= 0 && slime1.y < 550 )
            slime1.velY -= SLIME_JUMP_SPEED / 5;
        else if( slime1.velY <= 0 && slime1.y >= 550 )
            slime1.velY = 0;
        
        if( slime2.velY >= 0 && slime2.y < 550 )
            slime2.velY -= SLIME_JUMP_SPEED / 5;
        else if( slime2.velY <= 0 && slime2.y >= 550 )
            slime2.velY = 0;
        
        //keep ball in-bounds and bounce off window edges
        if( ball.y <= BALL_RADIUS ) {
            ball.y = BALL_RADIUS;
            ball.velY *= -1;
        } 
        if( ball.x <= BALL_RADIUS ) {
            ball.x = BALL_RADIUS;
            ball.velX *= -1;
        }
        else if( ball.x >= WINDOW_WIDTH - BALL_RADIUS ) {
            ball.x = WINDOW_WIDTH - BALL_RADIUS;
            ball.velX *= -1;
        }
        //GRAVITY: decrements ball velY every 5 ticks
        if( gravity > 3 ) {
            ball.velY -= 1;
            gravity = 0;
        }
         else
             gravity++;
        
       /**detect for collisions and change ball's velocity***/
        if( !detectCollision( &slime1, &ball ) ) //collision w/ slimes
            detectCollision( &slime2, &ball );
        
        //detect for ball impact on net
        if( ball.x <= (WINDOW_WIDTH / 2) + 8 + BALL_RADIUS && ball.x >= (WINDOW_WIDTH / 2) - 8 - BALL_RADIUS ) {
            //ball on top
            if( ball.velY < 0 && ball.y + BALL_RADIUS >= 420 && ball.y + BALL_RADIUS <= 430 )
                ball.velY *= -1;
            //ball hits side
            else if( ball.y + BALL_RADIUS > 420 )
                ball.velX *= -1;
        }
        
        //make ball adhere to max speed
        if( ball.velX > MAX_BALL_SPEED_X ) 
            ball.velX = MAX_BALL_SPEED_X;
        else if( ball.velX < -MAX_BALL_SPEED_X )
            ball.velX = -MAX_BALL_SPEED_X;
        
        if( ball.velY > MAX_BALL_SPEED_Y )
            ball.velY = MAX_BALL_SPEED_Y;
        else if( ball.velY < -MAX_BALL_SPEED_Y )
            ball.velY = -MAX_BALL_SPEED_Y;
        
        
        //100th second delay between frames TODO is this too fast?
        SDL_Delay( 10 );
  
    }
   
    
}



/**
 * draws a slime of specified size and location; should be called every frame
 * @param renderer - pointer to renderer
 * @param centerX - center of slime, x coord.
 * @param centerY - center of slime, y coord.
 * @param radius - radius of slime body
 */
void drawSlime( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius ) {
    
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while ( x >= y ) {
      //implements midpoint circle algorithm
      SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
      SDL_RenderDrawLine( renderer, centerX + x, centerY - y, centerX - x, centerY - y );
      SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
      
      SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
      SDL_RenderDrawLine( renderer, centerX + y, centerY - x, centerX - y, centerY - x );
      SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
      

      if (error <= 0) {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0) {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
      
   }
   
}



/**
 * drawSlime(...), except for a ball
 */
void drawBall( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius, SDL_Color edgeColor, SDL_Color fillColor ) {
    
    
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);
    
   while( x >= y ) {
       
      SDL_SetRenderDrawColor( renderer, edgeColor.r, edgeColor.g, edgeColor.b, edgeColor.a );
      SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
      SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
      SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
      SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
      SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
      SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
      SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
      SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
      
      SDL_SetRenderDrawColor( renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a );
      SDL_RenderDrawLine( renderer, centerX + x - 1, centerY - y + 1, centerX - x + 1, centerY - y + 1 );
      SDL_RenderDrawLine( renderer, centerX + x - 1, centerY + y - 1, centerX - x + 1, centerY + y - 1 );
      SDL_RenderDrawLine( renderer, centerX + y - 1, centerY - x + 1, centerX - y + 1, centerY - x + 1);
      SDL_RenderDrawLine( renderer, centerX + y - 1, centerY + x - 1, centerX - y + 1, centerY + x - 1);
      
      if (error <= 0) {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0) {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
      
   }
    
    
}


/**
 * checks for ball-on-slime collision and adjusts ball velocity if found
 * @param slime
 * @param ball
 * @return 1 if collision processed, 0 if no collision
 */
int detectCollision( struct movingObject * slime, struct movingObject * ball ) {
    
    int distX = slime->x - ball->x;
    int distY = slime->y - ball->y;
    int distance = (int)sqrt( (distX * distX) + (distY * distY) );
    
    int velDiffX = slime->velX - ball->velX;
    int velDiffY = slime->velY - ball->velY;
    
    //if there is no collision, return without modifying ball pos.
    if( distance > BALL_RADIUS + SLIME_RADIUS )
        return 0;
    
    int enigma = ( distX * velDiffX + distY * velDiffY ) / distance;
    if( enigma != 0 ) {
        
        ball->velX += slime->velX - ( 2 * distX * enigma / distance );
        ball->velY += slime->velY - ( 2 * distY * enigma / distance );
        ball->velY *= -1;
        
    }
    //******
    //if moving in same x or y direction, make sure they don't collide redundantly - this will cause stuttering
    if( slime->velX != 0 && ball->velX * 1.0 / slime->velX > 0.0 ) {
        if( abs( ball->velX ) < abs( slime->velX ) )
            ball->velX = slime->velX + (slime->velX / abs(slime->velX));
    }
    if( slime->velY != 0 && ball->velY * 1.0 / slime->velY > 0.0 ) {
        if( abs( ball->velY ) < abs( slime->velY ) )
            ball->velY = slime->velY + (slime->velY / abs(slime->velY));
    }
    //****/
    return 1;
    
}


/**
 * gives message upon point score
 * @param renderer because words need to be drawn to canvas
 * @param slimeNum slime 1 or 2, whichever scored on its opponent
 * @param font to write in
 */
void pointScored( SDL_Renderer * renderer, int slimeNum, TTF_Font * font ) {
    
    SDL_Color textColor = {0, 0, 0};
    
    SDL_Surface * goodMessageSurface;
    SDL_Texture * goodMessage;
    SDL_Rect goodRect;
    
    SDL_Surface * badMessageSurface;
    SDL_Texture * badMessage;
    SDL_Rect badRect;
    
    if( slimeNum == 1 ) {
        
        goodMessageSurface = TTF_RenderText_Solid( font, "Slime One Scores! Hooray", textColor );
        goodMessage = SDL_CreateTextureFromSurface( renderer, goodMessageSurface );
        
        goodRect.x = 100;
        goodRect.y = 575;
        goodRect.w = 300;
        goodRect.h = 50;
        
        badMessageSurface = TTF_RenderText_Solid( font, "Cringe! Slime Two Falters", textColor );
        badMessage = SDL_CreateTextureFromSurface( renderer, badMessageSurface );
        
        badRect.x = WINDOW_WIDTH - 400;
        badRect.y = 575;
        badRect.w = 300;
        badRect.h = 50;
        
    }
    
    else {
        
        badMessageSurface = TTF_RenderText_Solid( font, "Yikes! Slime One is salty", textColor );
        badMessage = SDL_CreateTextureFromSurface( renderer, badMessageSurface );
        
        badRect.x = 100;
        badRect.y = 575;
        badRect.w = 300;
        badRect.h = 50;
        
        goodMessageSurface = TTF_RenderText_Solid( font, "Slime Two Scores! Based", textColor );
        goodMessage = SDL_CreateTextureFromSurface( renderer, goodMessageSurface );
        
        goodRect.x = WINDOW_WIDTH - 400;
        goodRect.y = 575;
        goodRect.w = 300;
        goodRect.h = 50;
        
    }
    
    SDL_RenderCopy( renderer, goodMessage, NULL, &goodRect );
    SDL_RenderCopy( renderer, badMessage, NULL, &badRect );
    SDL_RenderPresent( renderer );
    
    SDL_Delay( 1500 ); //needs to be short or else game won't close
    
    SDL_FreeSurface( goodMessageSurface );
    SDL_DestroyTexture( goodMessage );
    SDL_FreeSurface( badMessageSurface );
    SDL_DestroyTexture( badMessage );
    
}


/**
 * prints victory message & ends game
 * @param renderer
 * @param slimeNum the winner
 * @param font
 */
void victoryAchieved( SDL_Renderer * renderer, int slimeNum, TTF_Font * font ) {
    
    SDL_Color textColor = {0, 0, 0};
    
    SDL_Surface * messageSurface;
    SDL_Texture * message;
    SDL_Rect rect;
    
    if( slimeNum == 1 )
        messageSurface = TTF_RenderText_Solid( font, "Slime 1 is victorious! You suck, blue!", textColor );
    else
        messageSurface = TTF_RenderText_Solid( font, "Slime 2 wins! Sad red will cry in corner.", textColor );
        
    message = SDL_CreateTextureFromSurface( renderer, messageSurface );
    rect.x = (WINDOW_WIDTH / 2) - 200;
    rect.y = 575;
    rect.w = 400;
    rect.h = 50;
    
    SDL_RenderCopy( renderer, message, NULL, &rect );
    SDL_RenderPresent( renderer );
    SDL_Delay( 2000 );
    
}


/**
 * 
 * @param oppsKeys Unit8 pointer to store key state
 * @param socket on which communicating with 
 */
void receiveOpponentKeyState( Uint8 * oppKeys, int socket, const Uint8 * myKeys ) {
    
    send( socket, myKeys, 8, 0 );
    recv( socket, oppKeys, 8, 0 );
    
}