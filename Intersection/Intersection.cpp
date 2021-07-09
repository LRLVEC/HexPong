#include <_Math.h>

struct LineSegment
{
	using vec2 = Math::vec2<double>;
	struct Intersection
	{
		using vec2 = Math::vec2<double>;

		bool intersected;
		double t1, t2;
		vec2 point;
		Intersection()
			:
			intersected(false),
			t1(0),
			t2(0),
			point{ 0 }
		{

		}
	};

	vec2 A, B;

	Intersection intersect(LineSegment b)
	{
		Intersection r;

		double l1((B - A).length()), l2((b.B - b.A).length());
		vec2 k1((B - A) / l1), k2((b.B - b.A) / l2);
		vec2 d(A - b.A);
		double s(k2[0] * k1[1] - k1[0] * k2[1]);
		if (s == 0)
		{
			r.intersected = false;
			return r;
		}
		r.t1 = (d[0] * k2[1] - k2[0] * d[1]) / s;
		r.t2 = (d[0] * k1[1] - k1[0] * d[1]) / s;
		r.point = (A + k1 * r.t1 + b.A + k2 * r.t2) / 2;
		if (r.t1 < 0 || r.t1 > l1 || r.t2 < 0 || r.t2 > l2)
			r.intersected = false;
		else
			r.intersected = true;
		return r;
	}
};

int main()
{
	using namespace Math;

	LineSegment AB{ {0,0},{1,1} };
	LineSegment CD{ {0,1},{2,0} };

	LineSegment::Intersection i(AB.intersect(CD));
	printf("%d", i.intersected);
	//i.point.print();
}