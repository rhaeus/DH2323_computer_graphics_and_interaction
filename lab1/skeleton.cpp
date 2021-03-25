// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Interpolate(float a, float b, std::vector<float>& result);
void TestFloatInterpolate();

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{
	TestFloatInterpolate();
	
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	while( NoQuitMessageSDL() )
	{
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Draw()
{

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{

		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 color(0,0,1);
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void TestFloatInterpolate()
{
	std::vector<float> result(10);
	Interpolate(5, 14, result);
	for (int i = 0; i < result.size(); ++i) {
		std::cout << result[i] << " ";
	}
}

void Interpolate(float a, float b, std::vector<float>& result) 
{
	if (result.size() == 0) {
		return;
	}

	if (result.size() == 1) {
		// return average  between a and b
		result[0] = (a + b) / 2.0;
	}

	int x0 = 0;
	int x1 = result.size() - 1;

	for (int i = 0; i < result.size(); ++i) {
		result[i] = (x1 - i) / float((x1 - x0)) * a + (i - x0) / float((x1 - x0)) * b;
	}
}