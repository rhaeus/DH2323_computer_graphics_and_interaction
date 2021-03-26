#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
SDL_Surface* screen;
int t;

std::vector<Triangle> triangles;
// float focalLength = 500.0f;
// glm::vec3 cameraPos(0, 0, -3.0f);

float focalLength = 2.0f * SCREEN_HEIGHT / 2.0f;
glm::vec3 cameraPos(0, 0, -(2.0f * focalLength / SCREEN_HEIGHT) - 1.0f);

// ----------------------------------------------------------------------------
// STRUCTS
struct Intersection
{
	glm::vec3 position;
	float distance;
	int triangleIndex;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

bool ClosestIntersection(glm::vec3 start, glm::vec3 dir, const std::vector<Triangle>& triangles, Intersection& closestIntersection);
bool RayTriangleIntersection(glm::vec3 start, glm::vec3 dir, const std::vector<Triangle>& triangles, Intersection& intersection);



int main( int argc, char* argv[] )
{
	LoadTestModel(triangles);

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

	Uint8* keystate = SDL_GetKeyState( 0 );
	if( keystate[SDLK_UP] )
	{
		// Move camera forward
		cameraPos.z += 0.1;
		std::cout << "camera forward" << std::endl;
	}
	if( keystate[SDLK_DOWN] )
	{
	// Move camera backward
		cameraPos.z -= 0.1;
		std::cout << "camera backward" << std::endl;
	}
	if( keystate[SDLK_LEFT] )
	{
	// Move camera to the left
		cameraPos.x -= 0.1;
	}
	if( keystate[SDLK_RIGHT] )
	{
	// Move camera to the right
		cameraPos.x += 0.1;
	}
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			glm::vec3 dir(x - SCREEN_WIDTH / 2.0f, y - SCREEN_HEIGHT / 2.0f, focalLength);
			dir = glm::normalize(dir);

			Intersection inter;

			if (ClosestIntersection(cameraPos, dir, triangles, inter)) {
				PutPixelSDL( screen, x, y, triangles[inter.triangleIndex].color);
			} else {
				PutPixelSDL( screen, x, y, glm::vec3(0, 0, 0) );
			}
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

bool RayTriangleIntersection(glm::vec3 start, glm::vec3 dir, const Triangle& triangle, int triangleIndex, Intersection& intersection)
{
	const glm::vec3& v0 = triangle.v0;
	const glm::vec3& v1 = triangle.v1;
	const glm::vec3& v2 = triangle.v2;

	const glm::vec3 e1 = v1 - v0;
	const glm::vec3 e2 = v2 - v0;
	const glm::vec3 b = start - v0;
	glm::mat3 A(-dir, e1, e2);
	glm::vec3 x = glm::inverse(A) * b; // x = (t u v)^T

	float t = x.x;
	float u = x.y;
	float v = x.z;

	bool intersect = u >= 0 && v >= 0 && (u + v) <= 1 && t >= 0;

	if (intersect) {
		intersection.distance = t;
		intersection.position = start + t * dir;
		intersection.triangleIndex = triangleIndex;
		return true;
	}

	return false;
}

bool ClosestIntersection(glm::vec3 start, glm::vec3 dir, const std::vector<Triangle>& triangles, Intersection& closestIntersection) 
{
	std::vector<Intersection> allIntersections;

	for (int i = 0; i < triangles.size(); ++i) {
		Intersection intersection;
		if (RayTriangleIntersection(start, dir, triangles[i], i, intersection)) {
			allIntersections.push_back(intersection);
		}
	}

	if (allIntersections.size() == 0) {
		// no intersection found
		return false;
	}

	// find closest intersection
	closestIntersection = allIntersections[0];

	for (int i = 0; i < allIntersections.size(); ++i) {
		if (allIntersections[i].distance < closestIntersection.distance) {
			closestIntersection = allIntersections[i];
		}
	}

	return true;
}