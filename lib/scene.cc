
#include <cassert>
#include <list>
using std::list;
#include <iostream>
using std::cerr;
using std::endl;

#include "scene.h"
#include "game.h"
#include "viewport.h"
#include "rect.h"
#include "actor.h"
#include "image.h"

namespace grail {

const std::string Scene::className = "Scene";

Scene::Scene() : _actorsMoved(false) {
}

Scene::Scene(Animation::ConstPtr background) : _actorsMoved(false) {
  this->background = background;
}

Scene::Scene(const std::string& backgroundPath) : _actorsMoved(false) {
  this->background = Animation::ConstPtr(new Image(backgroundPath));
}

Scene::~Scene() {
}

VirtualSize Scene::getSize() const {
  if(background) { return background->getSize(); }
  return VirtualSize(0, 0);
}

void Scene::setBackground(const Animation& background) {
  this->background.reset(&background);
}

void Scene::eachFrame(uint32_t ticks) {
  if(_actorsMoved) {
    actors.sort(Actor::CompareByY());
    _actorsMoved = false;
  }

  list<Actor::Ptr>::const_iterator iter;
  for(iter = actors.begin(); iter != actors.end(); iter++) {
    (*iter)->eachFrame(ticks);
  }
}

void Scene::renderAt(SDL_Surface* target, uint32_t ticks, VirtualPosition p) const {
  assert(target);
  if(background) {
    background->renderAt(target, ticks, p);
  }

  list<Actor::Ptr>::const_iterator iter;
  for(iter = actors.begin(); iter != actors.end(); iter++) {
    (*iter)->renderAt(target, ticks, p);
  }

} // renderAt

EventState Scene::handleEvent(SDL_Event& event, uint32_t ticks) {
  if(event.type == SDL_MOUSEBUTTONDOWN) {
    VirtualPosition pos = conv<SDL_MouseButtonEvent&, VirtualPosition>(event.button);

    if(Rect(getSize()).hasPoint(pos)) {
      list<Actor::Ptr>::const_iterator iter;
      bool event_sent = false;
      VirtualPosition cam = Game::getInstance().getViewport().getCameraPosition();

      for(iter = actors.begin(); iter != actors.end(); iter++) {
        if((*iter)->hasPoint(pos + cam)) {
          Event::actorClick(*iter, pos + cam, event.button.button)->push();

          event_sent = true;
          break;
        }
      } // for
      if(!event_sent) {
        Event::sceneClick(pos + cam, event.button.button)->push();
      }
    } // if area has point
  } // if mouse button down

  else if(event.type == SDL_MOUSEMOTION) {
    VirtualPosition pos = conv<SDL_MouseMotionEvent&, VirtualPosition>(event.motion);

    if(Rect(getSize()).hasPoint(pos)) {
      UserInterface::Ptr ui = Game::getInstance().getUserInterface();

      list<Actor::Ptr>::const_iterator iter;
      VirtualPosition cam = Game::getInstance().getViewport().getCameraPosition();

      bool hovered_something = false;
      for(iter = actors.begin(); iter != actors.end(); iter++) {
        if((*iter)->hasPoint(pos + cam)) {
          if(ui) {
            hovered_something = true;
            ui->setHovering(*iter);
            break;
          }
        }
      }
      if(!hovered_something) {
        ui->setHovering(Actor::Ptr());
      }
    } // if area has point
  } // if mouse motion


  return EVENT_STATE_UNHANDLED;
}

} // namespace grail

