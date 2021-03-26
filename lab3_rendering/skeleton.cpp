#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;

float focalLength = SCREEN_WIDTH;
glm::vec3 cameraPos(0, 0, -3.001);

glm::mat3 R(1, 0, 0, 0, 1, 0, 0, 0, 1);

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

void VertexShader(const glm::vec3& v, glm::ivec2& p);

int main( int argc, char* argv[] )
{
	LoadTestModel( triangles );
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	Uint8* keystate = SDL_GetKeyState(0);

	if( keystate[SDLK_UP] )
		;

	if( keystate[SDLK_DOWN] )
		;

	if( keystate[SDLK_RIGHT] )
		;

	if( keystate[SDLK_LEFT] )
		;

	if( keystate[SDLK_RSHIFT] )
		;

	if( keystate[SDLK_RCTRL] )
		;

	if( keystate[SDLK_w] )
		;

	if( keystate[SDLK_s] )
		;

	if( keystate[SDLK_d] )
		;

	if( keystate[SDLK_a] )
		;

	if( keystate[SDLK_e] )
		;

	if( keystate[SDLK_q] )
		;
}

void Draw()
{
	SDL_FillRect( screen, 0, 0 );

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	
	for( int i=0; i<triangles.size(); ++i )
	{
		vector<vec3> vertices(3);

		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;

		// Add drawing
		for (int v = 0; v < 3; ++v) {
			glm::ivec2 projPos;
			VertexShader(vertices[v], projPos);
			glm::vec3 color(1, 1, 1);
			PutPixelSDL(screen, projPos.x, projPos.y, color);
		}
	}
	
	if ( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void VertexShader(const glm::vec3& v, glm::ivec2& p) {
	auto p_t = (v - cameraPos) * R;

	p.x = focalLength * p_t.x / p_t.z + SCREEN_WIDTH / 2.0f;
	p.y = focalLength * p_t.y / p_t.z + SCREEN_HEIGHT / 2.0f;

}