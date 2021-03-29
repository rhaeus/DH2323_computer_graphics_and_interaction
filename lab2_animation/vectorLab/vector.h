#include <math.h>

namespace MyOGLProg
{

	// const double M_PI = 3.14159265358979323846;
	const double M_HALF_PI = 1.57079632679489661923;

	const double RAD = (M_PI/180.0);
	const double PIOVER2 = (3.14159265358979323846 / 2);
	#define DEG2RAD(x) ((x)*M_PI/180.0)
	#define RAD2DEG(x) ((x)*180.0/M_PI)


struct Position
{
	float x;
	float y;
	float z;
};

class Vector
{
	public:
		Vector(void);
		Vector(float x, float y, float z);
		Vector(Position& start, Position& end);
		~Vector(void){;}

		Vector addTo(const Vector &other) const;
		Vector subtractFrom(const Vector &other) const;
		
		float getMagnitude(void) const;
		void setMagnitude(const float m);
		Vector getCrossProduct(const Vector &other) const;

		float getDotProduct(const Vector &other) const;
		
		void normalise(void);		

		float x;
		float y;
		float z;
};


}