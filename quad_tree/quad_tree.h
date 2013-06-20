/*
	This software is provided AS IS without any guarantee about even  
	implied usefulness. It is NOT error free. It might and probably
	will destroy all your belongings. You can NOT sue me if that happens.

	You can use this software in any way you want given that you keep
	this disclaimer and the following copyright notice intact. If you
	change this software you are free to redistribute and ADD your own
	copyright notice below.

	copyright 2013 Florian Paul Schmidt (mista.tapas@gmx.net)
*/

#ifndef GREENFIELD_QUAD_TREE_HH
#define GREENFIELD_QUAD_TREE_HH

#include <boost/shared_ptr.hpp>
#include <set>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <utility>

namespace quad_tree
{
	/**
		@brief A class representing a quad tree used for spatial indexing. 
		
		NOTE: The requirements for class Point are such that there's an operator[] that can be used 
		to get the components (2) of the point.
		
		NOTE: This data structure is non intrusive. I.e. it keeps around only iterators to 
		an original data set. That also means that if you add any points they must be alive for 
		the whole life time of the quad_tree object otherwise its operations become undefined (all 
		except the destructor).
	*/
	template<class Point, class PointIterator, int NodeCapacity>
	struct quad_tree
	{
		typedef quad_tree<Point, PointIterator, NodeCapacity> quad_tree_t;
		
		typedef boost::shared_ptr<quad_tree_t> quad_tree_ptr;
		
		quad_tree_ptr m_north_west;
		
		quad_tree_ptr m_north_east;
		
		quad_tree_ptr m_south_west;
		
		quad_tree_ptr m_south_east;
		
		/*
			Represents the upper left and lower right corners
		*/
		typedef std::pair<Point, Point> Boundary;
		
		Boundary m_boundary;

		/*
			The points at this node
		*/
		typedef std::set<PointIterator> PointsSet;
		
		PointsSet m_points;
		
		
		/**
			This constructor determines the total bounding box by iterating over all points.
			
			NOTE: The iterator must be bidirectional.
			
			NOTE: The range of points must not be empty;
		*/	
		quad_tree
		(
			PointIterator points_begin,
			PointIterator points_end
		) 
		:
			m_level(0)
		{
			// std::cout << "quad_tree(it, it)" << std::endl;
			
			if (points_begin == points_end)
			{
				throw std::runtime_error("Empty range not permitted for this quad_tree constructor");
			}
			
			Boundary boundary(*points_begin, *points_begin);
			
			for(PointIterator it = points_begin; it != points_end; ++it)
			{
				const Point &p = *it;
				
				if (p[0] < boundary.first[0])
				{
					boundary.first[0] = p[0];
				}
				
				if (p[1] < boundary.first[1])
				{
					boundary.first[1] = p[1];
				}

				if (p[0] > boundary.second[0])
				{
					boundary.second[0] = p[0];
				}
				
				if (p[1] > boundary.second[1])
				{
					boundary.second[1] = p[1];
				}
			}
			
			m_boundary = boundary;
			
			check_boundary();
			
			add(points_begin, points_end);
		}
		
		quad_tree
		(
			const Boundary &boundary,
			PointIterator points_begin,
			PointIterator points_end
		) 
		:
			m_boundary(boundary),
			m_level(0)
		{
			// std::cout << "quad_tree(boundary, it, it) with boundary: " << boundary.first[0] << " " << boundary.first[1] << " " << boundary.second[0] << " " << boundary.second[1] << std::endl;
			
			check_boundary();
		}
		
		quad_tree
		(
			const Boundary &boundary
		) 
		:
			m_boundary(boundary),
			m_level(0)
		{
			// std::cout << "quad_tree(boundary) with boundary: " << boundary.first[0] << " " << boundary.first[1] << " " << boundary.second[0] << " " << boundary.second[1] << std::endl;
			
			check_boundary();
		}
		
		inline void check_boundary() const
		{
			if 
			(
				m_boundary.first[0] == m_boundary.second[0] ||
				m_boundary.first[1] == m_boundary.second[1]
			)
			{
				throw std::runtime_error("Degenerate boundary");
			}

			if 
			(
				m_boundary.first[0] > m_boundary.second[0] ||
				m_boundary.first[1] > m_boundary.second[1] 
			)
			{
				throw std::runtime_error("Order of boundary points not preserved");
			}
		}
		
		inline void add(PointIterator points_begin, PointIterator points_end)
		{
			for (PointIterator it = points_begin; it != points_end; ++it)
			{
				add(it);
			}
		}
		
		/**
			Returns true if the point was added to this node
			or one of its children
		*/
		inline bool add(PointIterator point_it)
		{
			if (false == point_intersects_boundary(*point_it))
			{
				return false;
			}
			
			if (true == has_children())
			{
				if (true == m_north_west->add(point_it))
				{
					return true;
				}
				
				if (true == m_north_east->add(point_it))
				{
					return true;
				}
				
				if (true == m_south_west->add(point_it))
				{
					return true;
				}
				
				if (true == m_south_east->add(point_it))
				{
					return true;
				}
				
				throw std::logic_error("This should not happen");
			}
			
			if (m_points.size() < NodeCapacity)
			{
				m_points.insert(point_it);
				return true;
			}
			
			split();
			
			return true;
		}
		
		inline bool has_children() const
		{
			assert
			(
				(m_north_west && m_north_east && m_south_west && m_south_east) ||
				(!m_north_west && !m_north_east && !m_south_west && !m_south_east)
			);
			
			// std::cout << "has_children: " << (bool)m_north_west << std::endl;
			
			return m_north_west;
		}
		
		inline bool point_intersects_boundary(const Point &point) const
		{
			const bool does_intersect = 
			(
				point[0] >= m_boundary.first[0] &&
				point[1] >= m_boundary.first[1] &&
				point[0] <= m_boundary.second[0] &&
				point[1] <= m_boundary.second[1]
			);	
			
			// std::cout << "point_intersects_boundary: " << does_intersect << std::endl;
			
			return does_intersect;
		}
		
		inline void split()
		{
			// std::cout << "split" << std::endl;
			
			Point center;

			center[0] = (m_boundary.second[0] + m_boundary.first[0]) / 2;
			center[1] = (m_boundary.second[1] + m_boundary.first[1]) / 2;
			
			const Point &upper_left = m_boundary.first;
			const Point &lower_right = m_boundary.second;
			
			/*
				North west
			*/
			m_north_west = quad_tree_ptr(new quad_tree_t(Boundary(upper_left, center)));

			m_north_west->m_level = m_level + 1;
			
			/*
				North east
			*/
			Point north_east_border_upper_left;

			north_east_border_upper_left[0] = center[0];
			north_east_border_upper_left[1] = upper_left[1];
			
			Point north_east_border_lower_right;

			north_east_border_lower_right[0] = lower_right[0];
			north_east_border_lower_right[1] = center[1];

			m_north_east = quad_tree_ptr
			(
				new quad_tree_t
				(
					Boundary(north_east_border_upper_left, north_east_border_lower_right)
				)
			);

			m_north_east->m_level = m_level + 1;

			/*
				South east
			*/
			m_south_east = quad_tree_ptr(new quad_tree_t(Boundary(center, lower_right)));

			m_south_east->m_level = m_level + 1;
			
			/*
				South west
			*/
			Point south_west_border_upper_left;

			south_west_border_upper_left[0] = upper_left[0];
			south_west_border_upper_left[1] = center[1];
			
			Point south_west_border_lower_right;

			south_west_border_lower_right[0] = center[0];
			south_west_border_lower_right[1] = lower_right[1];

					
			m_south_west = quad_tree_ptr
			(
				new quad_tree_t
				(
					Boundary(south_west_border_upper_left, south_west_border_lower_right)
				)
			);

			m_south_west->m_level = m_level + 1;
			
			/*
				Distribute points
			*/
			for (typename std::set<PointIterator>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				if (true == m_north_west->add(*it))
				{
					break;
				}
				
				if (true == m_north_east->add(*it))
				{
					break;
				}
				
				if (true == m_south_east->add(*it))
				{
					break;
				}
				
				if (true == m_south_west->add(*it))
				{
					break;
				}

				throw std::logic_error("This should not happen");
			}
			
			m_points.clear();
		}
		
		/**
			Used only by operator<< for formatting purposes
		*/
		int m_level;
	};

	void feed_spaces(int number_of_spaces, std::ostream &o)
	{
		for (int index = 0; index < number_of_spaces; ++index)
		{
			o << "  ";
		}
	}
	
	template<class Point, class PointIterator, int NodeCapacity>
	std::ostream &operator<<(std::ostream &o, const quad_tree<Point, PointIterator, NodeCapacity> &tree)
	{
		feed_spaces(tree.m_level, o);
		o << "Node [" << tree.m_boundary.first[0] << " " << tree.m_boundary.first[1] << "] [" << tree.m_boundary.second[0] << " " << tree.m_boundary.second[1] << "] => ( ";
		
		typedef quad_tree<Point, PointIterator, NodeCapacity> quad_tree_t;
		
		for (typename quad_tree_t::PointsSet::iterator it = tree.m_points.begin(); it != tree.m_points.end(); ++it)
		{
			o << "[" << (*(*it))[0] << " " << (*(*it))[1] << "] "; 
		}

		o << ")" << std::endl;
		
		if (true == tree.has_children())
		{
			o << *tree.m_north_west;
			o << *tree.m_north_east;
			o << *tree.m_south_east;
			o << *tree.m_south_west;
		}
		
		return o;
	}

} // namespace

#endif
