/*
   Small Program to Simulate a Numpad using a 2.4" TFT Touchscreen
   Program does not act as an USB HID Device, for reference purposes only
   Tested on Arduino UNO Only and 0x9341
   By William Tavares

   Note:
   This version is coplete with styling and numbers,
   if you want the smaller version get the "numpad-layout" program
   from my Github https://github.com/williamtavares/Arduino-Uno-NumPad

   Open the Serial Monitor to see the program running
   Enjoy!
*/

// #include "Adafruit_GFX.h"
#include "TouchScreen.h"
#include "Adafruit_TFTLCD.h"
#include "TimerOne.h"
#include "kclock.h"
#include "dgui.h"
#include "view.h"

#include "icons.h"

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// calibration mins and max for raw data when touching edges of screen
// YOU CAN USE THIS SKETCH TO DETERMINE THE RAW X AND Y OF THE EDGES TO GET YOUR HIGHS AND LOWS FOR X AND Y
#define TS_MAXX 897
#define TS_MINX 123

#define TS_MAXY 918
#define TS_MINY 91
char DEBUG[512];



//SPI Communication
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4


// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
// Pins A2-A6
// int rxplate = 364;
int rxplate = 0;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, rxplate);

//2.4 = 240 x 320
//Height 319 to fit on screen

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


#define WIDTH 240
#define HEIGHT 320



int cal_max_x = 0, cal_max_y= 0, cal_min_x = 600, cal_min_y= 600;

void calibrate() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //If sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (p.z == 0) {
    return;
  }
  cal_max_x = max(cal_max_x, p.x);
  cal_max_y = max(cal_max_y, p.y);

  cal_min_x = min(cal_min_x, p.x);
  cal_min_y = min(cal_min_y, p.y);


  PRINT("calibration: X [%i, %i] , Y [%i, %i]\n", cal_max_x, cal_min_x, cal_max_y, cal_min_y);
}

class CountersContainerView : public dgui::View {
public:
  CountersContainerView(const char* id, int x, int y, int w, int h) : dgui::View(id, x, y, w, h) {}
  void add(Component *child) {
    dgui::Component::add(child);

    int size = this->childs.size();

    int height = 4;

    int i = 0;
    for (SimpleList<Component *>::iterator itr = this->childs.begin(); itr != this->childs.end(); ++itr) {
      Component *component = (Component *)*itr;
      component->set_y(height * i++);
    }
    this->clear(&tft);
  }
};

class CounterView : public dgui::View {
public:
  CounterView(Counter *counter) : dgui::View("*CounterView"), left_btn("right_btn", 0, 0, 2, 2), text_comp("text", 3, 1 ,0 ,0) , right_btn("right_btn", 8, 0, 2, 2) {
    this->counter = counter;
    this->set_width(6);
    this->add(&this->text_comp);
    this->add(&this->left_btn);
    this->add(&this->right_btn);

    // this->right_btn.bitmap = &clock[0];
    this->left_btn.background_color = GREEN;
    this->right_btn.background_color = RED;

  }

  bool should_update() {
    unsigned long seconds = this->counter->seconds();
    if (current_time == seconds) return false;
    // PRINT("Update required");
    current_time = seconds;
    return true;
  }

  void render(Display * display) {
    this->counter->print(&this->time_str[0]);
    this->text_comp.set_text(&this->time_str[0]);
    dgui::View::render(display);
  }


private:
  Counter *counter;
  unsigned long current_time;
  char time_str[TIME_STR_LEN];
  dgui::Text text_comp;
  dgui::Button left_btn;
  dgui::Button right_btn;
};


dgui::View *main_view;
CountersContainerView *counters_view;


void on_click(dgui::Component *component) {
  PRINT("Click on %s\n", component->id());
  // component->debug();
  if (strcmp("add_counter", component->id()) == 0) {
    if (counters_count() >= MAX_COUNTERS) {
      return;
    }

    Counter *counter = new Counter();
    CounterView *view = new CounterView(counter);
    add_counter(counter);
    counters_view->add(view);
    PRINT("counter added");
  }
}


dgui::Component * current_target = NULL;
void retrieveTouch() {
  int height = tft.height();
  int width = tft.width();

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //If sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);



  if (p.z < 40) {
    if (current_target) {
      current_target = NULL;
      // PRINT("RESET TARGET: %i", p.z);
    }
    return;
  }



  int X = width - map(p.y, TS_MAXY, TS_MINY, 0, width);
  int Y = height - map(p.x, TS_MAXX, TS_MINX, 0, height);

  dgui::Component *target = main_view->find_click_target(X, Y);

  if (target == current_target) return;
  if (target != NULL) on_click(target);
  current_target = target;
}


void setup() {
  Serial.begin(9600);

  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3);

  Serial.print("Width: "); Serial.println(tft.width());
  Serial.print("Height: "); Serial.println(tft.height());
  tft.fillScreen(BLACK);

  setup_counters();


  main_view = new dgui::View("main_view");
  counters_view = new CountersContainerView("counters", 0, 0, 6, 4);
  main_view->add(counters_view);

  dgui::Button *add_counter = new dgui::Button("add_counter", 6, 0, 2, 2);
  add_counter->bitmap = &clock[0];
  add_counter->background_color = RED;
  main_view->add(add_counter);


  dgui::Button *add_timer = new dgui::Button("add_timer", 6, 6, 2, 2);
  add_timer->bitmap = &reloj_arena[0];
  add_timer->background_color = BLUE;
  main_view->add(add_timer);


  // main_view->debug();
  main_view->render(&tft);
}

int n = 0;



int page = 0, next_page = 0;

void loop() {

  // calibrate();
  retrieveTouch();
  main_view->render(&tft);
  // delay(200);
}
