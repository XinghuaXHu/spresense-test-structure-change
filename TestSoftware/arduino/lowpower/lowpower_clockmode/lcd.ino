
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

static inline void ycbcr2rgb(uint8_t y,  uint8_t cb, uint8_t cr,
                             uint8_t *r, uint8_t *g, uint8_t *b)
{
  int _r;
  int _g;
  int _b;
  _r = (128 * (y-16) +                  202 * (cr-128) + 64) / 128;
  _g = (128 * (y-16) -  24 * (cb-128) -  60 * (cr-128) + 64) / 128;
  _b = (128 * (y-16) + 238 * (cb-128)                  + 64) / 128;
  *r = itou8(_r);
  *g = itou8(_g);
  *b = itou8(_b);
}


/* Color conversion to show on display devices. */

static inline uint16_t ycbcrtorgb565(uint8_t y, uint8_t cb, uint8_t cr)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  
  ycbcr2rgb(y, cb, cr, &r, &g, &b);
  r = (r >> 3) & 0x1f;
  g = (g >> 2) & 0x3f;
  b = (b >> 3) & 0x1f;
  return (uint16_t)(((uint16_t)r << 11) | ((uint16_t)g << 5) | (uint16_t)b);
}

/* Color conversion to show on display devices. */

static void yuv2rgb(void *buf, uint32_t size)
{
  struct uyvy_s *ptr;
  struct uyvy_s uyvy;
  uint16_t *dest;
  uint32_t i;

  ptr = (uyvy_s*)buf;
  dest =(uint16_t*)buf;
  for (i = 0; i < size / 4; i++)
    {
      /* Save packed YCbCr elements due to it will be replaced with
       * converted color data.
       */

      uyvy = *ptr++;

      /* Convert color format to packed RGB565 */

      *dest++ = ycbcrtorgb565(uyvy.y0, uyvy.u0, uyvy.v0);
      *dest++ = ycbcrtorgb565(uyvy.y1, uyvy.u0, uyvy.v0);
    }
}


void display_lcd(uint16_t* buf, int16_t pos_x, int16_t pos_y, int width, int height, int img_size)
{
    int i;

    //Serial.println("Convert to RGB");
    //yuv2rgb(buf, img_size);
    
    // Set TFT address window to clipped image bounds
    tft.startWrite(); // Requires start/end transaction now
    tft.setAddrWindow(pos_x, pos_y, width, height);
    for( i=0; i< img_size/2; i++){
        tft.writePixel(buf[i]);
    }
    tft.endWrite(); // End last TFT transaction

    //Serial.println("Finish display");
}

void init_lcd(void)
{
    tft.begin();
    tft.fillScreen(ILI9341_BLUE);
    tft.setRotation(3);
}

