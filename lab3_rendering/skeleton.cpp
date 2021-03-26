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
float yaw = 0; // rotation angle around y axis
float pitch = 0; // rotation angle around x axis

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

void VertexShader(const glm::vec3& v, glm::ivec2& p);
void Interpolate(glm::ivec2 a, glm::ivec2 b, std::vector<glm::ivec2>& result);
void DrawLineSDL(SDL_Surface* surface, glm::ivec2 a, glm::ivec2 b, glm::vec3 color);
void DrawPolygonEdges(const std::vector<glm::vec3>& vertices);

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

	const float yaw_delta = 0.001;
	const float pitch_delta = 0.001;
	const float camera_delta = 0.005;

	Uint8* keystate = SDL_GetKeyState(0);
	if( keystate[SDLK_UP] )
	{
		// Move camera forward
		cameraPos.z += camera_delta;
	}
	if( keystate[SDLK_DOWN] )
	{
	// Move camera backward
		cameraPos.z -= camera_delta;
	}
	if( keystate[SDLK_LEFT] )
	{
	// rotate camera to the left
		yaw -= yaw_delta;
	}
	if( keystate[SDLK_RIGHT] )
	{
	// Move camera to the right
		yaw += yaw_delta;
	}

	glm::mat3 R_y = mat3(glm::cos(yaw), 0, glm::sin(yaw), 0, 1, 0, -glm::sin(yaw), 0, glm::cos(yaw));

	if( keystate[SDLK_RSHIFT] )
		;

	if( keystate[SDLK_RCTRL] )
		;

	if( keystate[SDLK_w] )
		cameraPos.z += camera_delta;

	if( keystate[SDLK_s] )
		cameraPos.z -= camera_delta;

	if( keystate[SDLK_d] )
		yaw -= yaw_delta;

	if( keystate[SDLK_a] )
		yaw += yaw_delta;

	if( keystate[SDLK_e] )
		pitch -= pitch_delta;

	if( keystate[SDLK_q] )
		pitch += pitch_delta;

	glm::mat3 R_x = mat3(1, 0, 0, 0, glm::cos(pitch), -glm::sin(pitch), 0, glm::sin(pitch), glm::cos(pitch));

	R = R_x * R_y;
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
		DrawPolygonEdges(vertices);
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

void Interpolate(glm::ivec2 a, glm::ivec2 b, std::vector<glm::ivec2>& result) {
	int N = result.size();
	glm::vec2 step = glm::vec2(b - a) / float(glm::max(N-1,1));
	glm::vec2 current(a);
	for (int i = 0; i < N; ++i) {
		result[i] = current;
		current += step;
	}
}

void DrawLineSDL(SDL_Surface* surface, glm::ivec2 a, glm::ivec2 b, glm::vec3 color) {
	glm::ivec2 delta = glm::abs(a - b);
	int pixels = glm::max(delta.x, delta.y) + 1;
	std::vector<glm::ivec2> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < line.size(); ++i) {
		PutPixelSDL(surface, line[i].x, line[i].y, color);
	}

}

void DrawPolygonEdges(const std::vector<glm::vec3>& vertices) {
	int V = vertices.size();
	// Transform vertices to 2D
	std::vector<glm::ivec2> projectedVertices(V);
	for (int i = 0; i < V; ++i) {
		VertexShader(vertices[i], projectedVertices[i]);
	}
	// draw edges
	for (int i = 0; i < V; ++i) {
		int j = (i+1) % V; // next vertex
		glm::vec3 color(1, 1, 1);
		DrawLineSDL(screen, projectedVertices[i], projectedVertices[j], color);
	}
}