// vim: set noexpandtab:

#ifndef GROUND_H
#define GROUND_H

#include <string>
#include <list>
using std::list;
#include <iostream>

#include "vector2d.h"
#include "polygon.h"
#include "actor.h" // ->Path
#include "line.h"
#include "utils.h" // ->sgn

namespace grail {

/**
 * The "ground" of a scene. This defines, from where you can go where and how.
 * 
 * The current pathfinding implementation is improveable performance-wise
 * (e.g. we don't use A*, only dijkstra, we don't pre-calculate a
 * shortest-path map, etc...) However for the expected number of waypoints
 * this will handle, this should be more than suffcient.
 * 
 * TODO: Don't allow users to place individual walls but only polygons.
 * After polygons have been placed, create one point for each side of a
 * corner.
 * 
 * TODO: Usability: Currently a potential user needs to be careful when to add
 * walls, when to add waypoints and when to generate the map. (E.g. adding
 * walls after that is mostly ineffective).
 * Provide assertions and documentation and/or more flexible behaviour to fix
 * this!
 */
class Ground {
	public:
		class Waypoint {
			public:
				typedef list<Waypoint*>::iterator NeighbourIterator;
				
			private:
				VirtualPosition position;
				list<Waypoint*> neighbours;
				double costSum;
				
				Waypoint(const Waypoint&) { }
				Waypoint& operator=(const Waypoint&) { return *this; }
				Waypoint(VirtualPosition position) : position(position), costSum(0) { }
				virtual ~Waypoint() { reset(); }
				
				void link(Waypoint& other) { if(likes(other.getPosition())) neighbours.push_back(&other); }
				void unlink(Waypoint& other) { neighbours.remove(&other); }
				
				// Due to walls overlapping at their endpoints and waypoint
				// doubling for reflecting both sides of a wall, we have
				// waypoint duplicates. Disallowing connections between those
				// avoids that they show up in pathfinding results
				bool likes(const Waypoint& other) { return other.position != position; }
				
			public:
				Waypoint* cheapestParent;
				
				struct GetPosition {
					static VirtualPosition getPosition(const Waypoint *wp) { return wp->getPosition(); }
				};
				
				void reset() {
					cheapestParent = 0;
					NeighbourIterator iter;
					for(iter = beginNeighbours(); iter != endNeighbours(); ++iter) {
						(*iter)->unlink(*this);
					}
				}
				
				VirtualPosition getPosition() const { return position; }
				bool operator==(const Waypoint& other) const { return position == other.position; }
				bool operator!=(const Waypoint& other) const { return position != other.position; }
				
				void linkBidirectional(Waypoint& other) {
					if(likes(other.getPosition()) && other.likes(position)) {
						link(other);
						other.link(*this);
					}
				}
				
				void setCostSum(double s) { costSum = s; }
				double getCostSum() { return costSum; }
				double costTo(Waypoint* other) { return (position - other->position).length(); }
				static int comparePointer(Waypoint* a, Waypoint* b) { return a->costSum - b->costSum; }
				NeighbourIterator beginNeighbours() { return neighbours.begin(); }
				NeighbourIterator endNeighbours() { return neighbours.end(); }
				friend class Ground;
				friend std::ostream& operator<<(std::ostream&, const Waypoint&);
		};
		
		class WallWaypoint : public Waypoint {
			private:
				WallWaypoint *next, *prev;
				int side; // -1=left, 1=right
				
				WallWaypoint(VirtualPosition position, int side) :
					Waypoint(position), next(0), prev(0), side(side) { }
				
				void setNext(WallWaypoint *n) { next = n; }
				void setPrev(WallWaypoint *p) { prev = p; }
				
				bool likes(const Waypoint& other) {
					return true;
					/*
					if(!Waypoint::likes(other)) { return false; }
					return
						(sgn((next.position - position).cross(other.position - position)) == side) &&
						(sgn((prev.position - position).cross(other.position - position
					*/
				}
				friend class Ground;
				friend std::ostream& operator<<(std::ostream&, const Waypoint&);
		};
		
	private:
		list<Line> walls;
		list<Waypoint*> waypoints;
		
		typedef Polygon<Waypoint*, Waypoint::GetPosition> WaypointPolygon;
		list<WaypointPolygon*> innerPolygons, outerPolygons;
		
		
	public:
		Ground();
		~Ground();
		
		/**
		 * Add walls from the edges of the given polygon.
		 */
		void addWalls(const Polygon<VirtualPosition, IsPosition>& polygon);
		
		const list<Line>& getWalls() const { return walls; }
		
		/**
		 * Call this after having added all needed walls.
		 * It will generate waypoints for all corners and connect them
		 * appropriately to allow pathfinding.
		 * Only call once or you will get unecessary waypoints.
		 */
		void generateMap();
		
		/**
		 * Add a new waypoint. Note that this will be pretty useless if you
		 * dont connect it to at least one other waypoint.
		 * Return the created Waypoint node.
		 * You might want to use waypoint.linkBidirectional(other_wp)
		 * to create a bidirectional link.
		 */
		//Waypoint& addWaypoint(VirtualPosition p);
		
		/**
		 * Returns true if point target can be reached in a direct line from source,
		 * ie there are no walls between them.
		 */
		bool directReachable(VirtualPosition source, VirtualPosition target);
		
		/**
		 * Returns a points of nodes that discribe a path from source to target.
		 */
		void getPath(VirtualPosition source, VirtualPosition target, Path& path);
		
		/// ditto.
		void getPath(Waypoint& source, Waypoint& target, Path& path);
};


#ifdef DEBUG
std::ostream& operator<<(std::ostream& os, const Ground::Waypoint& wp);
#endif // DEBUG


} // namespace grail

#endif // GROUND_H

