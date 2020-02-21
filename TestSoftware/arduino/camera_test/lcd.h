
#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library

#define TFT_DC 9
#define TFT_CS 6

#define itou8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

struct uyvy_s
{
  uint8_t u0;
  uint8_t y0;
  uint8_t v0;
  uint8_t y1;
};

struct v_buffer {
  uint32_t             *start;
  uint32_t             length;
};
typedef struct v_buffer v_buffer_t;

extern Adafruit_ILI9341 tft;

//Prototype
void display_lcd(uint16_t* buf, int16_t pos_x, int16_t pos_y, int width, int height, int img_size);
void init_lcd(void);
