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

const int STAR_COUNT = 1000;
std::vector<glm::vec3> stars(STAR_COUNT);

int t;
const float VELOCITY = 5.0f; //in m/s

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void DrawInterpolation();
void DrawStarfield();

void Interpolate(float a, float b, std::vector<float>& result);
void TestFloatInterpolate();

void Interpolate(glm::vec3 a, glm::vec3 b, std::vector<glm::vec3>& result);
void TestVec3Interpolate();

void Update();

// --------------------------------------------------------
// FUNCTION DEFINITIONS


int main( int argc, char* argv[] )
{
	TestFloatInterpolate();
	TestVec3Interpolate();

	// init star positions
	for(int i = 0; i < STAR_COUNT; ++i) {
		stars[i].x = float(rand()) / float(RAND_MAX) * 2.0 - 1.0;
		stars[i].y = float(rand()) / float(RAND_MAX) * 2.0 - 1.0;
		stars[i].z = float(rand()) / float(RAND_MAX);
	}

	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

	t = SDL_GetTicks();

	while( NoQuitMessageSDL() )
	{
		Update();
		DrawStarfield();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update() 
{
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t); //ms

	for (int s = 0; s < stars.size(); ++s) {
		// update stars
		stars[s].z -= VELOCITY * dt / 1000.0f; //use dt in seconds, velocity is in m/s

		// wrap stars 
		if (stars[s].z <= 0) {
			stars[s].z =+ 1;
		}

		if (stars[s].z > 1) {
			stars[s].z -= 1;
		}
	}
	t = t2;
}

void DrawStarfield()
{
	float f = SCREEN_HEIGHT / 2.0f;

	SDL_FillRect(screen, 0, 0);

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for (size_t s = 0; s < stars.size(); ++s) {
		int u = f * stars[s].x / stars[s].z + SCREEN_WIDTH / 2.0f;
		int v = f * stars[s].y / stars[s].z + SCREEN_HEIGHT / 2.0f;

		PutPixelSDL( screen, u, v, glm::vec3(1, 1, 1) );
	}


	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void DrawInterpolation()
{
	glm::vec3 topLeft(1,0,0); // red
	glm::vec3 topRight(0,0,1); // blue
	glm::vec3 bottomLeft(1,1,0); // yellow
	glm::vec3 bottomRight(0,1,0); // green

	std::vector<glm::vec3> leftSide(SCREEN_HEIGHT);
	std::vector<glm::vec3> rightSide(SCREEN_HEIGHT);
	Interpolate(topLeft, bottomLeft, leftSide);
	Interpolate(topRight, bottomRight, rightSide);

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		std::vector<glm::vec3> rowColors(SCREEN_WIDTH);
		Interpolate(leftSide[y], rightSide[y], rowColors);

		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			PutPixelSDL( screen, x, y, rowColors[x] );
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

	std::cout << std::endl;
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

void TestVec3Interpolate() {

	std::vector<glm::vec3> result( 4 );
	glm::vec3 a(1,4,9.2);
	glm::vec3 b(4,1,9.8);
	Interpolate( a, b, result );

	for( int i=0; i<result.size(); ++i )
	{
		cout << "( "
		<< result[i].x << ", "
		<< result[i].y << ", "
		<< result[i].z << " ) ";
	}

	std::cout << std::endl;
}

void Interpolate(glm::vec3 a, glm::vec3 b, std::vector<glm::vec3>& result) 
{
	if (result.size() == 0) {
		return;
	}

	if (result.size() == 1) {
		// return average  between a and b
		result[0].r = (a.r + b.r) / 2.0;
		result[0].g = (a.g + b.g) / 2.0;
		result[0].b = (a.b + b.b) / 2.0;
	}

	int x0 = 0;
	int x1 = result.size() - 1;

	for (int i = 0; i < result.size(); ++i) {
		result[i].r = (x1 - i) / float((x1 - x0)) * a.r + (i - x0) / float((x1 - x0)) * b.r;
		result[i].g = (x1 - i) / float((x1 - x0)) * a.g + (i - x0) / float((x1 - x0)) * b.g;
		result[i].b = (x1 - i) / float((x1 - x0)) * a.b + (i - x0) / float((x1 - x0)) * b.b;
	}

}