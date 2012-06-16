// vim: set noexpandtab:

#include <list>
#include "polygon.h"
#include "vector2d.h"
#include "debug.h"
#include "utils.h"

namespace grail {

template<typename Node, typename GetPosition>
Polygon<Node, GetPosition>::Polygon() : orientation(UNKNOWN) {
	printf("Constructing polygon %p\n", this);
}

template<typename Node, typename GetPosition>
Polygon<Node, GetPosition>::~Polygon() {
	printf("Destructing polygon %p\n", this);
}

template<typename Node, typename GetPosition>
bool Polygon<Node, GetPosition>::hasPoint(VirtualPosition p) const {
	if(hasBoundaryPoint(p)) {
		return true;
	}
	
	// If there is an uneven number of intersections
	// in both directions, the Offset is inside the
	// polygon
	bool unevenIntersectionsRight = false;
	bool unevenIntersectionsLeft = false;
	
	// Consider all lines from one point to the next one
	// plus one line from the last one to the first
	typename list<Node>::const_iterator iter, iter2;
	for(iter = nodes.begin(); iter != nodes.end(); iter++) {
		iter2 = iter;
		iter2++;
		if(iter2 == nodes.end()) { iter2 = nodes.begin(); }
		unevenIntersectionsRight ^= intersects(p, GetPosition::getPosition(*iter), GetPosition::getPosition(*iter2), true);
		unevenIntersectionsLeft ^= intersects(p, GetPosition::getPosition(*iter), GetPosition::getPosition(*iter2), false);
	}
	return unevenIntersectionsRight && unevenIntersectionsLeft;
}

template<typename Node, typename GetPosition>
bool Polygon<Node, GetPosition>::hasBoundaryPoint(VirtualPosition p) const {
	LineIterator li;
	for(li = beginLines(); li != endLines(); ++li) {
		if((*li).hasPoint(p)) { return true; }
	}
	return false;
}


template<typename Node, typename GetPosition>
typename Polygon<Node, GetPosition>::LineDirection Polygon<Node, GetPosition>::getLineDirection(Line l) const {
	
	/*
	 *         / l1
	 *        /
	 *       X ----->----- l
	 *       |\
	 *       | \
	 *       ^  \ l2
	 *       |
	 *       |
	 *      prev 
	 */
	
	LineIterator li;
	Line prev(GetPosition::getPosition(nodes.back()), GetPosition::getPosition(nodes.front()));
	Orientation o = getOrientation();
	
	for(li = beginLines(); li != endLines(); ++li) {
		if((*li).getA() == l.getA()) {
			VirtualPosition::Scalar s_l = ((*li).getB() - (*li).getA()).cross(l.getB() - l.getA());
			VirtualPosition::Scalar s_prev = (prev.getA() - prev.getB()).cross(l.getB() - l.getA());
			
			if(s_l == 0 || s_prev == 0) { return NEITHER; }
			
			if((o == CCW) && (s_l < 0) && (s_prev > 0)) { return IN; }
			else if((o == CW) && (s_l > 0) && (s_prev < 0)) { return IN; }
			else { return OUT; }
		} // if li.a == l.a
		prev = *li;
	} // for lines
	return NOT_ATTACHED;
} // getLineDirection()

template<typename Node, typename GetPosition>
bool Polygon<Node, GetPosition>::intersects(VirtualPosition q, VirtualPosition pa, VirtualPosition pb, bool right) {
	// Think a horizontal line drawn from q to the right and look
	// for an intersection.
	VirtualPosition::Y dy = pb.getY() - pa.getY();
	
	// If dy is 0, we test against a horizontal line. In this border-case we
	// define the lines as non-intersecting.
	if(dy == 0) {
		return false;
	}
	
	// We assumed endless straight lines up to now. Now look if the
	// intersection point is really between pa and pb.
	// t == 0 would mean: The intersection point is pa, t == 1 would mean the
	// intersection point is pb.

	double t = static_cast<double>(q.getY() - pa.getY()) / static_cast<double>(dy);
	if(t < 0.0 || t > 1.0) {
		return false;
	}
	
	if(t == 0.0 || t == 1.0) {
		// DARN! We hit a node. To make sure we didnt just strive the polygon,
		// consider it as intersection if the other point has a lower y-value.
		// (See http://alienryderflex.com/polygon/)
		
		if(!((t == 0.0 && pa.getY() > pb.getY()) || (t == 1.0 && pb.getY() > pa.getY()))) {
			return false;
		}
	}
	
	// w is the distance from q to the intersection point.
	// If it is not positive, we would have to draw our line to the left, but
	// we only consider one drawn to the right.
	VirtualPosition::X w = pa.getX() - q.getX() + (pb.getX() - pa.getX()) * t;
	if(right) {
		return w > 0;
	}
	else {
		return w < 0;
	}
} // intersects()

template<typename Node, typename GetPosition>
void Polygon<Node, GetPosition>::updateOrientation() const {
	LineIterator li = beginLines();
	double a = ((*li).getB() - (*li).getA()).getAngle(), new_a = 0, angleSum = 0;
	for(++li ; li != endLines(); ++li) {
		a = normalizeAngle(a);
		new_a = ((*li).getB() - (*li).getA()).getAngle();
		double x = a + M_PI - new_a;
		x = normalizeAngle(x);
		angleSum += x;
		a = new_a;
	}
	
	li = beginLines();
	new_a = ((*li).getB() - (*li).getA()).getAngle();
	a = normalizeAngle(a);
	double x = a + M_PI - new_a;
	x = normalizeAngle(x);
	angleSum += x;
	
	orientation = fequal(angleSum / (nodes.size() - 2.0), M_PI) ? CCW : CW;
}


#if VISUALIZE_POLYGONS
template<typename Node, typename GetPosition>
void Polygon<Node, GetPosition>::renderAt(SDL_Surface* target, uint32_t ticks, VirtualPosition p) const {
	for(LineIterator li = beginLines(); li != endLines(); ++li) {
		(*li).renderAt(target, ticks, p);
	}
}
#endif


} // namespace

