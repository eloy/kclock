#ifndef DGUI_H
#define DGUI_H

#include "kclock.h"
#include "SimpleList.h"

//class Adafruit_TFTLCD;
typedef Adafruit_TFTLCD Display;

namespace dgui {

  // const int SCALE_X = 320 / 8;
  // const int SCALE_Y = 240 / 4;

  typedef struct {
    int x, y, width, height = 0;
  } Dimension;



  // class Dimension {
  // public:

  //   Dimension(int x, int y, int width, int height) {
  //     this->x = x * SCALE_X;
  //     this-> y = y * SCALE_Y;
  //     this->width = width * SCALE_X;
  //     this->height = height * SCALE_Y;
  //   }

  //   void debug() {
  //     PRINT("Dimensions: %i, %i - [%ix%i]", this->x, this->y, this->width, this->height);
  //   }


  // };




  class Component {
  public:
    Component(const char* id) : _id(id), x(0), y(0), width(0), height(0) {
      this->parent = NULL;
      this->_should_update = true;
      this->resize();
    }

    Component(const char* id, int _x, int _y, int w, int h) : _id(id), x(_x), y(_y), width(w), height(h) {
      this->parent = NULL;
      this->_should_update = true;
      this->resize();
    }

    virtual void render(Display *display) {}
    virtual bool should_update() { return this->_should_update; };
    virtual void updated() { this->_should_update = false; };


    void add(Component *child) {
      child->parent = this;
      child->resize();
      this->childs.push_back(child);
    }

    void debug() {
      PRINT("Component %s: %i, %i - %ix%i (%i, %i - %i, %i) Childs: [", this->_id, this->x, this->y, this->width, this->height, this->dimension.x,this->dimension.y, this->dimension.width, this->dimension.height );
      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        PRINT(" %s ", component->id());
      }
      PRINT("]");

    }

    Component* find_click_target(int x, int y) {

      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        Component *target = component->find_click_target(x, y);
        if (target != NULL) return target;
      }

      this->debug();
      if (x < this->dimension.x || x > (this->dimension.x + this->dimension.width)) return NULL;
      if (y < this->dimension.y || y > (this->dimension.y + this->dimension.height)) return NULL;
      return this;
    }

    const char* id() { return this->_id; }

    void set_width(int w) { this->width = w; this->resize(); }
    void set_height(int h) { this->height = h; this->resize(); }
    void set_x(int x) { this->x = x; this->resize(); }
    void set_y(int y) { this->y = y; this->resize(); }

    void resize() {
      PRINT("Resizing %s", this->_id);
      int offset_x = 0, offset_y = 0;
      int parent_width, parent_height;

      if (this->parent != NULL) {
        offset_x = this->parent->dimension.x;
        offset_y = this->parent->dimension.y;
        parent_width = this->parent->dimension.width;
        parent_height = this->parent->dimension.height;
      } else {
        parent_width = 320;
        parent_height = 240;
      }

      int scale_x = parent_width / 8;
      int scale_y = parent_height / 4;


      PRINT("%i, %i, %i", this->x, offset_x, scale_x);
      PRINT("%i, %i, %i", this->y, offset_y, scale_y);

      this->dimension.x = (this->x * scale_x) + offset_x;
      this->dimension.y = (this->y * scale_y) + offset_y;

      this->dimension.width = this->width != 0 ? this->width * scale_x : parent_width;
      this->dimension.height = this->height != 0 ? this->height * scale_y : parent_height;

      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        component->resize();
      }
    }

    void clear(Display *display) {
      display->fillRect(this->dimension.x, this->dimension.y, this->dimension.width, this->dimension.height, 0);
    }



  protected:
    const char* _id;
    SimpleList<Component*> childs;
    Component *parent;
    bool _should_update;
    Dimension dimension;

    int x, y, width, height;
  };





  class View : public Component {
  public:
    View(const char* id) : Component(id, 0, 0, 0, 0) {}
    View(const char* id, int x, int y, int w, int h) : Component(id, x, y, w, h) {}

    bool should_update() { return true; }

    void render(Display *display) {
      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        if (component->should_update()) {
          component->render(display);
          component->updated();
        }
      }
    }

    Component* find_click_target(int x, int y) {
      this->debug();
      if (x < this->dimension.x || x > (this->dimension.x + this->dimension.width)) return NULL;
      if (y < this->dimension.y || y > (this->dimension.y + this->dimension.height)) return NULL;

      for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
        Component *component = (Component *)*itr;
        Component *target = component->find_click_target(x, y);
        if (target != NULL) return target;
      }
      return NULL;
    }

  };

  class Button : public Component {
  public:
    Button(const char* id) : Component(id) {}
    Button(const char* id, int x, int y, int w, int h) : Component(id, x, y, w, h) {}

    int background_color;

    void render(Display *display) {
      display->fillRect(this->dimension.x, this->dimension.y, this->dimension.width, this->dimension.height, this->background_color);
    }
  };




  class Text : public Component {
  public:
    Text(const char* id) : Component(id) {
      this->text = "";
    }
    Text(const char* id, int x, int y, int w, int h) : Component(id, x, y, w, h) {
      this->text = "";
    }
    void render(Display *display) {
      int x = this->dimension.x;
      int y = this->dimension.y;
      this->debug();
      display->fillRect(x, y, 50, 10, 0);
      display->setCursor(x, y);
      display->println(this->text);
    }

    void set_text(const char* text) {
      this->text = text;
      this->_should_update = true;
    }
  private:
    char* text;
  };

}





#endif //DGUI_H
