#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <U8g2lib.h>

/* default pins: specific to Heltec LoRa32 v3 board */
#define DEFAULT_OLED_SDA 17
#define DEFAULT_OLED_SCL 18
#define DEFAULT_OLED_RST 21
#define DEFAULT_OLED_VXT 36
#define DEFAULT_OLED_CLK_FREQ 400000

#define VEXT_ON(vxt)                                                           \
  pinMode(vxt, OUTPUT);                                                        \
  digitalWrite(vxt, LOW)

class OledDisplay {
private:
  uint8_t sda;
  uint8_t scl;
  uint8_t rst;
  uint8_t vxt;
  uint8_t mx_h = 0;    // max height
  uint8_t mx_w = 0;    // max width
  uint8_t char_h = 0;  // character height
  uint8_t char_w = 0;  // character width
  uint8_t mx_line = 0; // maximum lines
  uint8_t mx_cols = 0; // maximum cols
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;

public:
  OledDisplay()
      : oled(U8G2_R0, DEFAULT_OLED_RST, DEFAULT_OLED_SCL, DEFAULT_OLED_SDA),
        vxt(DEFAULT_OLED_VXT), rst(DEFAULT_OLED_RST), scl(DEFAULT_OLED_SCL),
        sda(DEFAULT_OLED_SDA) {} // Constructor with default pins values
  OledDisplay(uint8_t scl, uint8_t sda, uint8_t rst, uint8_t vext)
      : oled(U8G2_R0, rst, scl, sda), vxt(vext), rst(rst), scl(scl), sda(sda) {
  } // Constructor with pin arguments

  ~OledDisplay() {} // Default destructor

  /* Initialize the I2C comm interface */
  bool init();
  bool init(uint32_t freq);
  void clear();
  void write_px(uint8_t left, uint8_t top, const char *s);
  void write_px(uint8_t left, uint8_t top, String &s);
  void write(uint8_t col, uint8_t line, const char *s);
  void write(uint8_t col, uint8_t line, String &s);
  void clearLine(uint8_t line);
  void commit();
  void info();
};

#endif // DISPLAY_H_
