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

float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

glm::vec3 lightPos(0, -0.5f, -0.7f);
glm::vec3 lightPower = 5.1f * glm::vec3(1, 1, 1);
glm::vec3 indirectLightPowerPerArea = 0.5f * glm::vec3(1, 1, 1);

glm::vec3 currentNormal;
glm::vec3 currentReflectance;

struct Pixel 
{
	glm::ivec2 position;
	float zinv;
	glm::vec3 pos3d;
};

struct Vertex
{
	glm::vec3 position;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

void VertexShader(const Vertex& v, Pixel& p);

void Interpolate(Pixel a, Pixel b, std::vector<Pixel>& result);

void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b);

void DrawPolygonEdges(const std::vector<glm::vec3>& vertices);

void ComputePolygonRows(const std::vector<Pixel>& vertexPixels, std::vector<Pixel>& leftPixels, std::vector<Pixel>& rightPixels);

void DrawPolygonRows(const std::vector<Pixel>& leftPixels, const std::vector<Pixel>& rightPixels);

void DrawPolygon(const std::vector<Vertex>& vertices);

void PixelShader(const Pixel& p);

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
	// init depth buffer
	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			depthBuffer[y][x] = 0;
		}
	}
	SDL_FillRect( screen, 0, 0 );

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	
	for( int i=0; i<triangles.size(); ++i )
	{
		currentReflectance = triangles[i].color;
		currentNormal = triangles[i].normal;

		std::vector<Vertex> vertices(3);
		vertices[0].position = triangles[i].v0;
		vertices[1].position = triangles[i].v1;
		vertices[2].position = triangles[i].v2;

		DrawPolygon(vertices);
	}
	
	if ( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void VertexShader(const Vertex& v, Pixel& p) {
	auto p_t = (v.position - cameraPos) * R;

	p.zinv = 1.0f / p_t.z;
	p.position.x = focalLength * p_t.x * p.zinv + SCREEN_WIDTH / 2.0f;
	p.position.y = focalLength * p_t.y * p.zinv + SCREEN_HEIGHT / 2.0f;
	p.pos3d = v.position;
}

void Interpolate(Pixel a, Pixel b, std::vector<Pixel>& result) {
	int N = result.size();
	glm::vec2 step_position = glm::vec2(b.position - a.position) / float(glm::max(N-1,1));
	float step_z = float(b.zinv - a.zinv) / float(glm::max(N-1,1));
	glm::vec3 step_pos3d = (b.pos3d - a.pos3d) / float(glm::max(N-1,1));

	glm::vec2 current_position(a.position);
	float current_z = a.zinv;
	glm::vec3 current_pos3d(a.pos3d);

	for (int i = 0; i < N; ++i) {
		// std::cout << current.x  << ", " << current.y << " | " << glm::round(current).x << ", " << glm::round(current).y << std::endl;
		result[i].position = glm::round(current_position);
		current_position += step_position;

		result[i].zinv = current_z;
		current_z += step_z;

		result[i].pos3d = current_pos3d;
		current_pos3d += step_pos3d;
	}

}

void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b) {
	glm::ivec2 delta = glm::abs(a.position - b.position);
	int pixels = glm::max(delta.x, delta.y) + 1;
	std::vector<Pixel> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < line.size(); ++i) {
		PixelShader(line[i]);
	}
}

void ComputePolygonRows(const std::vector<Pixel>& vertexPixels, std::vector<Pixel>& leftPixels, std::vector<Pixel>& rightPixels) {
	// 1. Find max and min y-value of the polygon
	// and compute  the number or rows it occupies.
	int min_y = std::numeric_limits<int>::max();
	int max_y = std::numeric_limits<int>::min();
	for (int i = 0; i < vertexPixels.size(); ++i) {
		if (vertexPixels[i].position.y > max_y) {
			max_y = vertexPixels[i].position.y;
		}

		if (vertexPixels[i].position.y < min_y) {
			min_y = vertexPixels[i].position.y;
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
		leftPixels[i].position.x = std::numeric_limits<int>::max();
		leftPixels[i].position.y = min_y + i;
		rightPixels[i].position.x = std::numeric_limits<int>::min();
		rightPixels[i].position.y = min_y + i;
	}

	// 4. Loop through all edges of the polygon and use
	// linear interpolation to find the x-coordinate for
	// each row it occupies. Update the corresponding
	// values in rightPixels and leftPixels
	int V = vertexPixels.size();
	for (int i = 0; i < V; ++i) {
		int j = (i+1) % V; // next vertex

		glm::ivec2 delta = glm::abs(vertexPixels[i].position - vertexPixels[j].position);
		int pixels = glm::max(delta.x, delta.y) + 1;
		std::vector<Pixel> line(pixels);
		Interpolate(vertexPixels[i], vertexPixels[j], line);

		for (int k = 0; k < line.size(); ++k) {
			int r = line[k].position.y - min_y;

			// if (r < 0 || r >= rows) {
			// 	std::cout << "huch " << line[k].y << " " << min_y <<  std::endl;
			// }

			if (line[k].position.x < leftPixels[r].position.x) {
				leftPixels[r].position.x = line[k].position.x;
				leftPixels[r].position.y = line[k].position.y;
				leftPixels[r].zinv = line[k].zinv;
				leftPixels[r].pos3d = line[k].pos3d;
			}

			if (line[k].position.x > rightPixels[r].position.x) {
				rightPixels[r].position.x = line[k].position.x;
				rightPixels[r].position.y = line[k].position.y;
				rightPixels[r].zinv = line[k].zinv;
				rightPixels[r].pos3d = line[k].pos3d;
			}
		}

	}
}

void DrawPolygonRows(const std::vector<Pixel>& leftPixels, const std::vector<Pixel>& rightPixels) {
	for (int i = 0; i < leftPixels.size(); ++i) {
		DrawLineSDL(screen, leftPixels[i], rightPixels[i]);
	}
}

void DrawPolygon(const std::vector<Vertex>& vertices) {
	int V = vertices.size();
	std::vector<Pixel> vertexPixels(V);
	for (int i = 0; i < V; ++i) {
		VertexShader(vertices[i], vertexPixels[i]);
	}

	std::vector<Pixel> leftPixels;
	std::vector<Pixel> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
}

void PixelShader(const Pixel& p) {
	int x = p.position.x;
	int y = p.position.y;
	if (p.zinv > depthBuffer[y][x]) {
		depthBuffer[y][x] = p.zinv;
		glm::vec3 r = lightPos - p.pos3d;
		float r_squared = r.x * r.x + r.y * r.y + r.z * r.z;

		glm::vec3 D = lightPower * glm::max(glm::dot(glm::normalize(r), currentNormal), 0.0f) / float(4.0f * M_PI * r_squared);
		auto illumination = currentReflectance * (D + indirectLightPowerPerArea);
		PutPixelSDL(screen, x, y, illumination);
	}
}
