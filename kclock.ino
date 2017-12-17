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
#include "font.h"
#include "kclock.h"
#include "view.h"
#include "dgui.h"

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// calibration mins and max for raw data when touching edges of screen
// YOU CAN USE THIS SKETCH TO DETERMINE THE RAW X AND Y OF THE EDGES TO GET YOUR HIGHS AND LOWS FOR X AND Y
//calibration: X [863, 156] , Y [914, 113]
// calibration: X [897, 123] , Y [918, 91]
#define TS_MAXX 897
#define TS_MINX 123

#define TS_MAXY 918
#define TS_MINY 91
char DEBUG[512];


// #define TS_MAXX 914
// #define TS_MINX 113

// #define TS_MAXY 863
// #define TS_MINY 156


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

int x, y;
Counter counter;

void drawXBitmap(int16_t x, int16_t y, int index, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
  int16_t i, j;
  int byteWidth = (font_width + 7) / 8;
  int offset = ((index * 20) + 7) / 8;
  byteWidth = 28;

  for(j=0; j < h; j++) {
    for(i=0; i < w; i++ ) {
      if(pgm_read_byte(bitmap + ((j * byteWidth) + (i / 8)) + offset) & (1 << (i % 8))) {
        tft.drawPixel(x+i, y+j, color);
      } else {
        tft.drawPixel(x+i, y+j, BLACK);
      }
    }
  }
}



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

dgui::View main_view("main_view");


void on_click(dgui::Component *component) {
  PRINT("Selected component: %s\n", component->id());
}


dgui::Component * current_target = NULL;
int min_z = 800;
void retrieveTouch() {
  int height = tft.height();
  int width = tft.width();

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //If sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);



  if (p.z < 10) {
    if (current_target) {
      current_target = NULL;
      // PRINT("RESET TARGET: %i", p.z);
    }
    return;
  }

  // min_z = min(min_z, p.z);
  // PRINT("Reading pressure: %i [min: %i]", p.z, min_z);

  int X, Y;

  X = width - map(p.y, TS_MAXY, TS_MINY, 0, width);
  Y = height - map(p.x, TS_MAXX, TS_MINX, 0, height);
  // PRINT("Converted: %i, %i, %i\n", X, Y, p.z);

  dgui::Component *target = main_view.find_click_target(X, Y);

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
  x = 0;
  y = 0;

  setup_counters();
  add_counter(&counter);
  // drawXBitmap(0, 0, 0, &font_bits[0], 20, 25, RED);


  dgui::Button *btn = new dgui::Button("add", 6, 0, 2, 2);
  btn->background_color = RED;
  main_view.add(btn);

  btn->render(&tft, 0, 0);

  // draw_main_layout(&tft);
}

int n = 0;


char buffer[2][TIME_STR_LEN];
int page = 0;

void loop() {

  counter.print(&buffer[page][0]);

  if (strcmp(&buffer[0][0], &buffer[1][0]) != 0) {
    tft.fillRect(0, 0, 50, 10, BLACK);
    tft.setCursor(0, 0);
    tft.println(&buffer[page][0]);
    page = page == 0 ? 1 : 0;
  }

  // calibrate();
  retrieveTouch();

  // delay(200);
}
