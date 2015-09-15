#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include "shared.hpp"
#include "arduino-serial-lib.hpp"
//#include <SDL2/SDL_ttf.h>

void die(int code);
int serialPort = -1;

int main( int argc, char * argv[] )
{	
	SDL_Window * window;
	SDL_Renderer * renderer;
	{
		SDL_Init( SDL_INIT_VIDEO );
		window = SDL_CreateWindow("UGV keyboard",
		                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		                          600, 200, 
		                          SDL_WINDOW_RESIZABLE );
		assert( window != NULL );
		renderer = SDL_CreateRenderer( window, -1, 0 );
		assert( renderer != NULL );
		//TTF_Init();
	}
	
	serialPort = serialport_init("/dev/ttyUSB0", 115200);
	serialport_write(serialPort, "[+40+40:BB]");
	
	
	//TTF_Font * font;
	//font = TTF_OpenFont( "font.ttf", winw/40 );
	
	// Motor speeds
	char * leftMotor;
	char * rightMotor;
	
	leftMotor = "+00";
	rightMotor = "+00";
	
	while ( true )
	{
		SDL_Event e;
		if( SDL_PollEvent( &e ) != 0 && e.type == SDL_KEYDOWN )
		{
			switch (e.key.keysym.sym)
			{
				case '4': leftMotor = "+FF"; break;
				case 'r': leftMotor = "+60"; break;
				case 'f': leftMotor = "+00"; break;
				case 'v': leftMotor = "-60"; break;
				
				case '5': rightMotor = "+FF"; break;
				case 't': rightMotor = "+60"; break;
				case 'g': rightMotor = "+00"; break;
				case 'b': rightMotor = "-60"; break;
				
				case 'q': die(0); break;
				default: break;
			}
		}
		
		
		// Synthesize message
		char message[10];
		sprintf( message, "%s%s:", leftMotor, rightMotor );
		int checksum = calculateChecksum( message, 7 );
		char finalmessage[20];
		sprintf(finalmessage,"[%s%c%c]", message, decToHex(checksum / 16), decToHex(checksum % 16) );
		serialport_write(serialPort, finalmessage);
		printf("%s\n", finalmessage );
		
		SDL_Delay( 50 );
		
		// Update screen
		SDL_SetRenderDrawColor( renderer, 0, 0, 0,  255);
		SDL_RenderClear( renderer);
		SDL_RenderPresent( renderer);
	}
	
	return 0;
}

void die(int code)
{
	//TTF_Quit();
	serialport_close(serialPort);
	SDL_Quit();
	exit(code);
}
