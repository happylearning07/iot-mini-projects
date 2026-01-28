#include "display.h"
#include <Arduino.h>
#include <Wire.h>

bool OledDisplay::init() { return this->init(DEFAULT_OLED_CLK_FREQ); }

bool OledDisplay::init(uint32_t freq) {
  VEXT_ON(vxt);
  delay(100);
  if (!Wire.begin(sda, scl, freq))
    return false;
  pinMode(rst, OUTPUT);
  digitalWrite(rst, LOW);
  delay(50);
  digitalWrite(rst, HIGH);

  oled.begin();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.clearBuffer();

  const char *msg = "Initializing...";

  // obtain size parameters of the display
  mx_h = oled.getDisplayHeight();
  mx_w = oled.getDisplayWidth();
  char_w = oled.getMaxCharWidth();
  char_h = oled.getMaxCharHeight();
  mx_line = mx_h / char_h;
  mx_cols = mx_w / char_w;

  // text width in pixels
  int16_t text_w = oled.getStrWidth(msg);

  // vertical center using font ascent/descent
  int16_t text_h = oled.getAscent() - oled.getDescent();

  int16_t x = (mx_w - text_w) / 2;
  int16_t y = (mx_h + text_h) / 2;

  this->write_px(x, y, msg); // Write the message in the
                             // center of the display
  oled.sendBuffer();
  return true;
}

void OledDisplay::clear() { oled.clearDisplay(); }

void OledDisplay::write_px(uint8_t left, uint8_t top, const char *s) {
  oled.drawStr(left, top, s);
}

void OledDisplay::write_px(uint8_t left, uint8_t top, String &s) {
  oled.drawStr(left, top, s.c_str());
}

void OledDisplay::write(uint8_t col, uint8_t line, const char *s) {
  auto x = col * char_w;
  auto y = (line + 1) * char_h;
  write_px(x, y, s);
}

void OledDisplay::write(uint8_t col, uint8_t line, String &s) {
  auto x = col * char_w;
  auto y = (line + 1) * char_h;
  write_px(x, y, s);
}

void OledDisplay::clearLine(uint8_t line) {
  auto y = line * char_h;
  oled.setDrawColor(0);
  oled.drawBox(0, y, mx_w, char_h);
  oled.setDrawColor(1);
}

void OledDisplay::commit() { oled.sendBuffer(); }

void OledDisplay::info() {
  Serial.printf("OledDisplay Object:\n");
  Serial.printf("----------------------\n");
  Serial.printf("SCL: %d\n", scl);
  Serial.printf("SDA: %d\n", sda);
  Serial.printf("RST: %d\n", rst);
  Serial.printf("VEXT: %d\n", vxt);
  Serial.printf("Max. Height: %d\n", mx_h);
  Serial.printf("Max. Width: %d\n", mx_w);
  Serial.printf("Char Height: %d\n", char_h);
  Serial.printf("Char Width: %d\n", char_w);
  Serial.printf("Max line count: %d\n", mx_line);
  Serial.printf("Max cols count: %d\n", mx_cols);
}
