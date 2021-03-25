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

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;

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
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 color( 1, 0.5, 0.5 );
			PutPixelSDL( screen, x, y, color );
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

	bool intersect = u > 0 && v > 0 && (u + v) < 1 && t >= 0;

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
	// float closestDist = std::numeric_limits<float>::max();
	closestIntersection = allIntersections[0];

	for (int i = 0; i < allIntersections.size(); ++i) {
		if (allIntersections[i].distance < closestIntersection.distance) {
			closestIntersection = allIntersections[i];
		}
	}

	return true;
}