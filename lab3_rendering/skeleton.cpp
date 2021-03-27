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

glm::vec3 currentColor(0, 0, 0);

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

void VertexShader(const glm::vec3& v, glm::ivec2& p);
void Interpolate(glm::ivec2 a, glm::ivec2 b, std::vector<glm::ivec2>& result);
void DrawLineSDL(SDL_Surface* surface, glm::ivec2 a, glm::ivec2 b, glm::vec3 color);
void DrawPolygonEdges(const std::vector<glm::vec3>& vertices);
void ComputePolygonRows(const std::vector<glm::ivec2>& vertexPixels, std::vector<glm::ivec2>& leftPixels, std::vector<glm::ivec2>& rightPixels);
void TestComputePolygonRows();
void DrawPolygonRows(const std::vector<glm::ivec2>& leftPixels, const std::vector<glm::ivec2>& rightPixels);
void DrawPolygon(const std::vector<glm::vec3>& vertices);

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
	// int dx;
	// int dy;
	// SDL_GetRelativeMouseState(&dx, &dy);
	// yaw += dx * 0.001f;
	// pitch += dy * 0.001f;

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
		currentColor = triangles[i].color;
		vector<vec3> vertices(3);
		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;
		DrawPolygon(vertices);
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
		// std::cout << current.x  << ", " << current.y << " | " << glm::round(current).x << ", " << glm::round(current).y << std::endl;
		result[i] = glm::round(current);
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

void ComputePolygonRows(const std::vector<glm::ivec2>& vertexPixels, std::vector<glm::ivec2>& leftPixels, std::vector<glm::ivec2>& rightPixels) {
	// 1. Find max and min y-value of the polygon
	// and compute  the number or rows it occupies.
	int min_y = std::numeric_limits<int>::max();
	int max_y = std::numeric_limits<int>::min();
	for (int i = 0; i < vertexPixels.size(); ++i) {
		if (vertexPixels[i].y > max_y) {
			max_y = vertexPixels[i].y;
		}

		if (vertexPixels[i].y < min_y) {
			min_y = vertexPixels[i].y;
		}
	}
	int rows = max_y - min_y + 1;

	// 2. Resize leftPixels and rightPixels
	// so that they have an element for each row.
	leftPixels.resize(rows);
	rightPixels.resize(rows);

	// 3. Initialize the x-coordinates in leftPixels
	// to some really large value and the x-coordinates 
	// in rightPixels to some really small value
	for (int i = 0; i < rows; ++i) {
		leftPixels[i].x = std::numeric_limits<int>::max();
		leftPixels[i].y = min_y + i;
		rightPixels[i].x = std::numeric_limits<int>::min();
		rightPixels[i].y = min_y + i;
	}

	// 4. Loop through all edges of the polygon and use
	// linear interpolation to find the x-coordinate for
	// each row it occupies. Update the corresponding
	// values in rightPixels and leftPixels
	int V = vertexPixels.size();
	for (int i = 0; i < V; ++i) {
		int j = (i+1) % V; // next vertex

		glm::ivec2 delta = glm::abs(vertexPixels[i] - vertexPixels[j]);
		int pixels = glm::max(delta.x, delta.y) + 1;
		std::vector<glm::ivec2> line(pixels);
		Interpolate(vertexPixels[i], vertexPixels[j], line);

		for (int k = 0; k < line.size(); ++k) {
			int r = line[k].y - min_y;

			// if (r < 0 || r >= rows) {
			// 	std::cout << "huch " << line[k].y << " " << min_y <<  std::endl;
			// }

			if (line[k].x < leftPixels[r].x) {
				leftPixels[r].x = line[k].x;
				leftPixels[r].y = line[k].y;
			}

			if (line[k].x > rightPixels[r].x) {
				rightPixels[r].x = line[k].x;
				rightPixels[r].y = line[k].y;
			}
		}

	}
}

void TestComputePolygonRows() {
	std::vector<glm::ivec2> vertexPixels(3);
	vertexPixels[0] = glm::ivec2(10, 5);
	vertexPixels[1] = glm::ivec2(5, 10);
	vertexPixels[2] = glm::ivec2(15, 15);
	std::vector<glm::ivec2> leftPixels;
	std::vector<glm::ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);

	for (int row = 0; row < leftPixels.size(); ++row) {
		std::cout << "Start: ("
		<< leftPixels[row].x << ","
		<< leftPixels[row].y << "). "
		<< "End: ("
		<< rightPixels[row].x << ","
		<< rightPixels[row].y << "). " << std::endl;
	}
}

void DrawPolygonRows(const std::vector<glm::ivec2>& leftPixels, const std::vector<glm::ivec2>& rightPixels) {
	for (int i = 0; i < leftPixels.size(); ++i) {
		DrawLineSDL(screen, leftPixels[i], rightPixels[i], currentColor);
	}
}

void DrawPolygon(const std::vector<glm::vec3>& vertices) {
	int V = vertices.size();
	std::vector<glm::ivec2> vertexPixels(V);
	for (int i = 0; i < V; ++i) {
		VertexShader(vertices[i], vertexPixels[i]);
	}

	std::vector<glm::ivec2> leftPixels;
	std::vector<glm::ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
	
}