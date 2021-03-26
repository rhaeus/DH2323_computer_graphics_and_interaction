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

std::vector<Triangle> triangles;
// float focalLength = 500.0f;
// glm::vec3 cameraPos(0, 0, -3.0f);

float focalLength = 2.0f * SCREEN_HEIGHT / 2.0f;
glm::vec3 cameraPos(0, 0, -(2.0f * focalLength / SCREEN_HEIGHT) - 1.0f);

glm::mat3 R;
float yaw = 0.0f; // rotation angle aroung y axis in rad

glm::vec3 lightPos(0, -0.5f, -0.7f);
glm::vec3 lightColor = 14.0f * glm::vec3(1, 1, 1); // power P, energy per time unit in W = J/s

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

glm::vec3 DirectLight(const Intersection& i);


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
	}
	if( keystate[SDLK_DOWN] )
	{
	// Move camera backward
		cameraPos.z -= 0.1;
	}
	if( keystate[SDLK_LEFT] )
	{
	// rotate camera to the left
		yaw -= 0.05;
	}
	if( keystate[SDLK_RIGHT] )
	{
	// Move camera to the right
		yaw += 0.05;
	}

	R = mat3(glm::cos(yaw), 0, glm::sin(yaw), 0, 1, 0, -glm::sin(yaw), 0, glm::cos(yaw));

	if (keystate[SDLK_w]) {
		lightPos.z += 0.1;
	}
	if (keystate[SDLK_s]) {
		lightPos.z -= 0.1;
	}
	if (keystate[SDLK_d]) {
		lightPos.x += 0.1;
	}
	if (keystate[SDLK_a]) {
		lightPos.x -= 0.1;
	}
	if (keystate[SDLK_e]) {
		lightPos.y += 0.1;
	}
	if (keystate[SDLK_q]) {
		lightPos.y -= 0.1;
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
				// glm::vec3 color = triangles[inter.triangleIndex].color;
				glm::vec3 D = DirectLight(inter);
				glm::vec3 color = D * triangles[inter.triangleIndex].color;
				PutPixelSDL( screen, x, y, color);
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
	const glm::vec3& v0 = R * triangle.v0;
	const glm::vec3& v1 = R * triangle.v1;
	const glm::vec3& v2 = R * triangle.v2;

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

glm::vec3 DirectLight(const Intersection& i)
{
	glm::vec3 triangleLight = lightPos - i.position;
	float distToLightSquared = triangleLight.x * triangleLight.x + triangleLight.y * triangleLight.y + triangleLight.z * triangleLight.z;

	glm::vec3 B = lightColor / float(4.0f * M_PI * distToLightSquared);

	glm::vec3 D = B * glm::max(glm::dot(glm::normalize(lightPos - i.position), glm::normalize(triangles[i.triangleIndex].normal)), 0.0f);

	return D;
}