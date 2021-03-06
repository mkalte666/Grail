// vim: set noexpandtab:

#ifndef SCENE_H
#define SCENE_H

#include <algorithm>
#include <string>

#include <boost/shared_ptr.hpp>

#include "classes.h"
#include "vector2d.h"
#include "animation.h"
#include "event.h"
#include "actor.h"
#include "ground.h"

namespace grail {

class Scene {
		struct Parallax {
			Animation::Ptr animation;
			VirtualPosition offset;
			double scrollFactorX, scrollFactorY;
			Parallax(Animation::Ptr animation, VirtualPosition offset, double scrollFactorX, double scrollFactorY) :
				animation(animation), offset(offset), scrollFactorX(scrollFactorX), scrollFactorY(scrollFactorY) {
			}
			unsigned int id;
		}; // Parallax
		
		Animation::Ptr background;
		VirtualSize size;
		std::list<Parallax*> backgrounds, foregrounds;
		std::list<Actor::Ptr> actors;
		bool _actorsMoved;
		Ground ground;
		bool _drawWalls;
		unsigned int background_id_counter;
		unsigned int getBackgroundId();
		
	public:
		typedef boost::shared_ptr<Scene> Ptr;
		
		static const std::string className;
		
		//Scene();
		Scene(VirtualSize sz);
		Scene(Animation::Ptr);
		Scene(const std::string& backgroundPath);
		
		Ground& getGround() { return ground; }
		
		virtual ~Scene();
		
		VirtualSize getSize() const;
		
		void setBackground(Animation::Ptr background);
		
		unsigned int addBackground(Animation::Ptr background, VirtualPosition offset = VirtualPosition(), double scrollFactorX = 1.0, double scrollFactorY = 1.0);
		unsigned int addForeground(Animation::Ptr forgeground, VirtualPosition offset = VirtualPosition(), double scrollFactorX = 1.0, double scrollFactorY = 1.0);

		void delBackground(unsigned int id);
		void delForeground(unsigned int id);
		
		void addActor(Actor::Ptr entity) {
			actors.push_back(entity);
			std::inplace_merge(actors.begin(), actors.end(), actors.end(), Actor::CompareByY());
		}
		void removeActor(Actor::Ptr entity);
		void actorsMoved() { _actorsMoved = true; }
		void enableDrawWalls(bool t=true) { _drawWalls = t; }
		
		void eachFrame(uint32_t ticks);
		void renderAt(SDL_Surface* target, uint32_t ticks, VirtualPosition p) const;
		
		EventState handleEvent(SDL_Event& event, uint32_t ticks);
		
		virtual void onEnter() { }
};

} // namespace grail

#endif // SCENE_H

