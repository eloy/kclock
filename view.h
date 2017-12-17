#ifndef VIEW_H
#define VIEW_H


//Color Definitons
#define BLACK     0x0000
#define BLUE      0x001F
#define GREY      0xCE79
#define LIGHTGREY 0xDEDB
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF



class Adafruit_TFTLCD;

void draw_main_layout(Adafruit_TFTLCD *tft);

#endif //VIEW_H
