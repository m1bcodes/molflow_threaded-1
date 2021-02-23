#include "FracturePoly.h"

#include <vector>
#include <fstream>

#include <boost/polygon/polygon.hpp>

namespace bp = boost::polygon;
using CoordType = int; // ClipperLib::cInt;
using SimplePolygon = bp::polygon_data<CoordType>;
using Point = bp::point_data<CoordType>;
using ComplexPolygon = bp::polygon_with_holes_data<CoordType>;
using PolygonSet = bp::polygon_set_data<CoordType>;
using SimplePolygons = std::vector<bp::polygon_data<CoordType>>;

using namespace boost::polygon::operators;

//void x()
//{
//	if (nrhs < 2) throw std::exception("At least three arguments required.");
//	ClipperLib::Paths in1, in2;
//	getPaths(prhs[1], in1);
//	parseOptions(prhs, nrhs, 2);
//
//	//typedef boost::polygon::polygon_with_holes_data<int> polygon;
//	//typedef boost::polygon::polygon_traits<polygon>::point_type point;
//	//typedef boost::polygon::polygon_with_holes_traits<polygon>::hole_type hole;
//
//	//point pts[5] = {
//	//boost::polygon::construct<point>(0, 0),
//	//boost::polygon::construct<point>(0, 10),
//	//boost::polygon::construct<point>(10, 10),
//	//boost::polygon::construct<point>(10, 0),
//	//boost::polygon::construct<point>(0, 0)
//	//};
//	//point hole_pts[5] = {
//	//	boost::polygon::construct<point>(1, 1),
//	//	boost::polygon::construct<point>(9, 1),
//	//	boost::polygon::construct<point>(9, 9),
//	//	boost::polygon::construct<point>(1, 9),
//	//	boost::polygon::construct<point>(1, 1)
//	//};
//
//	//hole hls[1];
//	//boost::polygon::set_points(hls[0], hole_pts, hole_pts + 5);
//
//	//polygon poly;
//	//boost::polygon::set_points(poly, pts, pts + 5);
//	//boost::polygon::set_holes(poly, hls, hls + 1);
//
//	//for (ClipperLib::Path& p : in1) {
//	//	if (!ClipperLib::Orientation(p))
//	//		ClipperLib::ReversePath(p);
//	//}
	//namespace bp = boost::polygon;
	//using SimplePolygon = bp::polygon_data<int>;
	//using ComplexPolygon = bp::polygon_with_holes_data<int>;
	//using Point = bp::point_data<int>;
	//using PolygonSet = bp::polygon_set_data<int>;
	//using SimplePolygons = std::vector<bp::polygon_data<int>>;

	//using namespace boost::polygon::operators;

	//ComplexPolygon p;
	//PolygonSet complexPolygons;

	//std::vector<SimplePolygon> holes;
	//SimplePolygon outer;
	//bool outerFound = false;
	//for (ClipperLib::Path& path : in1) {
	//	std::vector<Point> points; // { {5, 0}, { 10, 5 }, { 5, 10 }, { 0, 5 } };
	//	for (ClipperLib::IntPoint pt : path)
	//		points.push_back(Point(pt.X, pt.Y));
	//	if (ClipperLib::Orientation(path)) {
	//		if (outerFound) throw std::exception("More than one outer polygon");
	//		outer.set(points.begin(), points.end());
	//		outerFound = true;
	//	}
	//	else {
	//		SimplePolygon sp;
	//		sp.set(points.begin(), points.end());
	//		holes.push_back(sp);
	//	}
	//}
	//bp::set_points(p, outer.begin(), outer.end());
	//bp::set_holes(p, holes.begin(), holes.end());

	//complexPolygons += p;

	//SimplePolygons simplePolygons;
	//complexPolygons.get<SimplePolygons>(simplePolygons);
//}

SimplePolygon simplePolyFromPath(const ClipperLib::Path& path)
{
	std::vector<Point> points;
	for (size_t ip = 0; ip < path.size(); ip++)
	{
		Point pt(path[ip].X, path[ip].Y);
		points.push_back(pt);
	}
	SimplePolygon sp;
	sp.set(points.begin(), points.end());
	return sp;
}

void recurseClipperTree(const ClipperLib::PolyNode* n, ClipperLib::Paths& paths)
{
	if (!n->IsHole()) {
		std::vector<SimplePolygon> holes;
		SimplePolygon outer = simplePolyFromPath(n->Contour);

		for (int ic = 0; ic < n->ChildCount(); ic++) {
			const ClipperLib::PolyNode* child = n->Childs[ic];
			if (child->IsHole()) {
				holes.push_back(simplePolyFromPath(child->Contour));
			}
			else {
				throw std::runtime_error("Strange topology: non-hole is child of non-hole");
			}
		}
		ComplexPolygon cp;
		PolygonSet complexPolygons;

		bp::set_points(cp, outer.begin(), outer.end());
		bp::set_holes(cp, holes.begin(), holes.end());
		
		complexPolygons += cp;

		SimplePolygons simplePolygons;
		complexPolygons.get<SimplePolygons>(simplePolygons);

		for (SimplePolygon sp : simplePolygons)
		{
			ClipperLib::Path p;
			for (Point pt : sp) p.push_back(ClipperLib::IntPoint(pt.x(), pt.y()));
			paths.push_back(p);
		}
	}

	for (int ic = 0; ic < n->ChildCount(); ic++) {
		const ClipperLib::PolyNode* child = n->Childs[ic];
		recurseClipperTree(child, paths);
	}
}

void processClipperSolution(const ClipperLib::PolyTree& solution, ClipperLib::Paths& paths)
{
	int childCount = solution.ChildCount();
	for (int ic = 0; ic < childCount; ic++) {
		ClipperLib::PolyNode* child = solution.Childs[ic];
		recurseClipperTree(child, paths);
	}

	for(size_t ip = 0; ip < paths.size(); ip++) {
		const ClipperLib::Path& p = paths[ip];
		std::ofstream ofs("diff_" + std::to_string(ip) + ".txt");
		for (const ClipperLib::IntPoint& pt : p) {
			ofs << pt.X << ", " << pt.Y << "\n";
		}
	}
}
