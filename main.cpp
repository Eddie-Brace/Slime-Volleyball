/* 
 * File:   main.cpp
 * Author: Edward Brace
 * A desktop recreation of Quinn Pendragon's classic 'Slime Volleyball' game.
 */

#include <cstdlib>
#include <SDL2/SDL.h>

#define SPEED_X 5
#define SPEED_Y 50
#define SLIME_RADIUS 75

using namespace std;
void drawSlime( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius );
void drawBall( SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius );

struct movingObject {  int x;   int y;    int velX;  int velY; };


/*
 * this runs the game; currently no params
 */
int main(int argc, char** argv) {
    //start SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 ) {
        printf( "Error initializing SDL: %s", SDL_GetError() );
        return 1;
    }
    //create window
    SDL_Window * window = SDL_CreateWindow( "Slime Volleyball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 650, 0 );
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
    
    
    //need structs for both slimes & ball
    struct movingObject slime1;
        slime1.x = 320;
        slime1.y = 550;
        slime1.velX = 0;
        slime1.velY = 0;
    struct movingObject slime2;
        slime2.x = 880;
        slime2.y = 550;
        slime2.velX = 0;
        slime2.velY = 0;
    struct movingObject ball;
        ball.x = 320;
        ball.y = 350;
        ball.velX = 0;
        ball.velY = 0; //change this

        
    /***
     * now that all the init work is complete, is time to animate and interact w/ user in loop until game end
     */
    while( 1 ) {
    
        //draw blue sky background and grass playing field
        SDL_SetRenderDrawColor(renderer, 0, 230, 255, 255 );
        SDL_RenderClear( renderer );
    
        SDL_Rect grass;
            grass.x = 0;
            grass.y = 550;
            grass.w = 1200;
            grass.h = 100;
        SDL_SetRenderDrawColor( renderer, 0, 200, 30, 255 );
        SDL_RenderFillRect( renderer, &grass );
        
        
        //move slimes to new position
        slime1.x += slime1.velX;
        slime1.y -= slime1.velY;
        slime2.x += slime2.velX;
        slime2.y -= slime2.velY;
        //prevent from going out of bounds
        if( slime1.y > 550 )
            slime1.y = 550;
        if( slime2.y > 550 )
            slime2.y = 550;
        
        if( slime1.x < SLIME_RADIUS )
            slime1.x = SLIME_RADIUS - 1;
        else if( slime1.x > 592 - SLIME_RADIUS)
            slime1.x = 592 - SLIME_RADIUS;
        
        if( slime2.x > 1200 - SLIME_RADIUS )
            slime2.x = 1200 - SLIME_RADIUS;
        else if( slime2.x < 608 + SLIME_RADIUS )
            slime2.x = 608 + SLIME_RADIUS - 1;
    
        
        //draw slimes, net, & ball
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        drawSlime( renderer, slime1.x, slime1.y, SLIME_RADIUS );
        SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
        drawSlime( renderer, slime2.x, slime2.y, SLIME_RADIUS );
    
        SDL_Rect net;
            net.x = 592;
            net.y = 420;
            net.w = 16;
            net.h = 130;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255 );
        SDL_RenderFillRect( renderer, &net );
    
        drawBall( renderer, 320, 350, 25 );
    
    
        //print graphics to visible buffer
        SDL_RenderPresent( renderer );
    
        //close program on window exit
        SDL_Event e;
        SDL_PollEvent( &e );
        if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE )
            exit(0);
        //read keys to interpret players movement
        const Uint8 * keys = SDL_GetKeyboardState( NULL );
        
        if( keys[SDL_SCANCODE_W] && slime1.y == 550)
            slime1.velY = SPEED_Y;
        if( keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D] )
            slime1.velX = -SPEED_X;
        else if( keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime1.velX = SPEED_X;
        else if( !keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A] )
            slime1.velX = 0;
            
        if( keys[SDL_SCANCODE_U] && slime2.y == 550 )
            slime2.velY = SPEED_Y;
        if( keys[SDL_SCANCODE_H] && !keys[SDL_SCANCODE_K] )
            slime2.velX = -SPEED_X;
        else if( keys[SDL_SCANCODE_K] && !keys[SDL_SCANCODE_H] )
            slime2.velX = SPEED_X;
        else if( !keys[SDL_SCANCODE_K] && !keys[SDL_SCANCODE_H] )
            slime2.velX = 0;
        
        
        //take gravity and ground force into account
        if( slime1.velY >= 0 && slime1.y < 550 )
            slime1.velY = -SPEED_Y / 6;
        else if( slime1.velY <= 0 && slime1.y >= 550 )
            slime1.velY = 0;
        
        if( slime2.velY >= 0 && slime2.y < 550 )
            slime2.velY = -SPEED_Y / 6;
        else if( slime2.velY <= 0 && slime2.y >= 550 )
            slime2.velY = 0;
        
        //100th second delay between frames TODO is this too fast? not reading key inputs?
        SDL_Delay( 10 );
  
    }
    
    //clean up and end program
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
    
    return 0;
    
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

   while (x >= y) {
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
void drawBall(SDL_Renderer * renderer, int32_t centerX, int32_t centerY, int32_t radius) {
    
    
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

