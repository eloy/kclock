#ifndef DGUI_H
#define DGUI_H

#include "kclock.h"
#include "SimpleList.h"

//class Adafruit_TFTLCD;
typedef Adafruit_TFTLCD Display;

namespace dgui {

  const int SCALE_X = 320 / 8;
  const int SCALE_Y = 240 / 4;
  class Dimension {
  public:
    Dimension(int x, int y, int width, int height, int offset_x, int offset_y) {
      x = x + offset_x;
      y = y + offset_y;

      this->x = x * SCALE_X;
      this-> y = y * SCALE_Y;
      this->width = width * SCALE_X;
      this->height = height * SCALE_Y;
    }

    void debug() {
      PRINT("Dimensions: %i, %i - [%ix%i]", this->x, this->y, this->width, this->height);
    }

    int x, y, width, height;
  };


  class Component {
  public:
    Component(const char* id) : _id(id), x(0), y(0), width(8), height(4) {}
    Component(const char* id, int _x, int _y, int w, int h) : _id(id), x(_x), y(_y), width(w), height(h) {}
    virtual void render(Display *display, int offset_x, int offset_y) {}
    const int x, y, width, height;
    void add(Component *child) {
      this->childs.push_back(child);
    }

    void debug() {
      PRINT("Component %s: %i, %i - [%ix%i]", this->_id, this->x, this->y, this->width, this->height);
    }

    Component* find_click_target(int x, int y) {
      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        Component *target = component->find_click_target(x, y);
        if (target != NULL) return target;
      }

      Dimension d(this->x, this->y, this->width, this->height, 0, 0);
      if (x < d.x || x > (d.x + d.width)) return NULL;
      if (y < d.y || y > (d.y + d.height)) return NULL;
      return this;
    }
    const char* id() {
      return this->_id;
    }
  private:
    const char* _id;

    SimpleList<Component*> childs;
  };

  class View : public Component {
  public:
    View(const char* id) : Component(id) {}
    View(const char* id, int x, int y, int w, int h) : Component(id, x, y, w, h) {}

  };

  class Button : public Component {
  public:
    Button(const char* id) : Component(id) {}
    Button(const char* id, int x, int y, int w, int h) : Component(id, x, y, w, h) {}

    int background_color;

    void render(Display *display, int offset_x, int offset_y) {
      Dimension d(this->x, this->y, this->width, this->height, offset_x, offset_y);
      d.debug();
      display->fillRect(d.x, d.y, d.width, d.height, this->background_color);
    }
  };
}





#endif //DGUI_H
