/* 
 * File:   main.cpp
 * Author: Edward Brace
 * A desktop recreation of Quinn Pendragon's classic 'Slime Volleyball' game.
 */

#include <cstdlib>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1300
#define SLIME_WALK_SPEED 11
#define SLIME_JUMP_SPEED 40
#define SLIME_RADIUS 75
#define BALL_RADIUS 25
#define MAX_BALL_SPEED_X 13
#define MAX_BALL_SPEED_Y 10

using namespace std;
void drawSlime( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius );
void drawBall( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius );
int detectCollision( struct movingObject * slime, struct movingObject * ball );
void exitProgram( int sig_id );

struct movingObject {  int x;   int y;    int velX;  int velY; };

/*
 * this runs the game; currently no params
 */
int main(int argc, char** argv) {
    //start SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 ) //mandatory error checking
    {
        printf( "Error initializing SDL: %s", SDL_GetError() );
        return 1;
    }
    //create window
    SDL_Window * window = SDL_CreateWindow( "Slime Volleyball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, 650, 0 );
    if( !window ) //mandatory error checking
    {
        printf( "Error creating window: %s", SDL_GetError() );
        SDL_Quit();
        return 1;
    }
    
    //create graphics renderer
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer * renderer = SDL_CreateRenderer( window, -1, render_flags );
    if( !renderer ) //mandatory error checking
    {
        printf( "Error creating renderer: %s", SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }
    
    start:
    //need structs for both slimes & ball
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
        ball.x = WINDOW_WIDTH / 4;
        ball.y = 350;
        ball.velX = 0;
        ball.velY = -4; //change this
    int gravity = 0;

        
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
    
        drawBall( renderer, ball.x, ball.y, BALL_RADIUS );
    
    
        //print graphics to visible buffer
        SDL_RenderPresent( renderer );
    
        //close program on window exit
        SDL_Event e;
        SDL_PollEvent( &e );
        if( e.type == SDL_QUIT || e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE ) {
            //clean up and end program
            SDL_DestroyRenderer( renderer );
            SDL_DestroyWindow( window );
            SDL_Quit();
    
            return 0;
        }
        /**code for point on ball->ground impact here
         */
        if( ball.y >= 550 - BALL_RADIUS ) {
            goto start;
        }
        
        //read keys to interpret players movement
        const Uint8 * keys = SDL_GetKeyboardState( NULL );
        
        if( keys[SDL_SCANCODE_W] && slime1.y == 550)
            slime1.velY = SLIME_JUMP_SPEED;
        if( keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D] )
            slime1.velX = -SLIME_WALK_SPEED;
        else if( keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime1.velX = SLIME_WALK_SPEED;
        else if( !keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime1.velX = 0;
            
        if( keys[SDL_SCANCODE_U] && slime2.y == 550 )
            slime2.velY = SLIME_JUMP_SPEED;
        if( keys[SDL_SCANCODE_H] && !keys[SDL_SCANCODE_K] )
            slime2.velX = -SLIME_WALK_SPEED;
        else if( keys[SDL_SCANCODE_K] && !keys[SDL_SCANCODE_H] )
            slime2.velX = SLIME_WALK_SPEED;
        else if( !keys[SDL_SCANCODE_K] && !keys[SDL_SCANCODE_H] )
            slime2.velX = 0;
        
        
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
        if( ball.x <= 608 + BALL_RADIUS && ball.x >= 592 - BALL_RADIUS ) {
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
void drawBall( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius ) {
    
    
    const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);
    
   while( x >= y ) {
       
      SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
      SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
      SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
      SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
      SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
      SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
      SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
      SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
      SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
      
      SDL_SetRenderDrawColor( renderer, 245, 245, 245, 255 );
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
 * checks for ball-on-slime collision and adjust ball velocity if found
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
    
    return 1;
    
}