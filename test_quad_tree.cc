#include <quad_tree/quad_tree.h>

#include <boost/array.hpp>
#include <vector>
#include <iostream>

#include <unistd.h>

#include <boost/timer/timer.hpp>

int main()
{
	boost::timer::auto_cpu_timer t;

	typedef boost::array<float, 2> Point;
	typedef std::vector<Point>::iterator PointIterator;
	
	const int capacity = 4;
	
	std::vector<Point> points;
	
	for (size_t index = 0; index < 100098; ++index)
	{
		Point p;
		
		p[0] = (int)(100 * (float)rand()/RAND_MAX);
		p[1] = (int)(100 * (float)rand()/RAND_MAX);
		
		points.push_back(p);
	}
	
	Point p1;
	
	p1[0] = 0;
	p1[1] = 0;
	
	points.push_back(p1);

	Point p2;
	
	p2[0] = 100;
	p2[1] = 100;
	
	points.push_back(p2);

	typedef quad_tree::point_quad_tree<Point, PointIterator, capacity> quad_tree;
		
	quad_tree tree(points.begin(), points.end());
	
	std::cout << tree << std::endl;

	std::cout << tree.number_of_points() << std::endl;
}
