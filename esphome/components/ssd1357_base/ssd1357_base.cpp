#include "ssd1357_base.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::ssd1357_base {

static const char *const TAG = "ssd1357";

static const uint16_t SSD1357_COLORMASK = 0xffff;
static const uint8_t SSD1357_MAX_CONTRAST = 15;
static const uint8_t SSD1357_BYTESPERPIXEL = 2;
// SSD1357 commands
static const uint8_t SSD1357_SETCOLUMN = 0x15;
static const uint8_t SSD1357_SETROW = 0x75;
static const uint8_t SSD1357_SETREMAP = 0xA0; //Different
static const uint8_t SSD1357_STARTLINE = 0xA1;
static const uint8_t SSD1357_DISPLAYOFFSET = 0xA2;
static const uint8_t SSD1357_DISPLAYOFF = 0xAE;
static const uint8_t SSD1357_DISPLAYON = 0xAF;
static const uint8_t SSD1357_PRECHARGE = 0xB1; //Different
static const uint8_t SSD1357_CLOCKDIV = 0xB3;  //Different
static const uint8_t SSD1357_PRECHARGELEVEL = 0xBB; //Different
static const uint8_t SSD1357_VCOMH = 0xBE;
// display controls
static const uint8_t SSD1357_DISPLAYALLOFF = 0xA4;
static const uint8_t SSD1357_DISPLAYALLON = 0xA5;
static const uint8_t SSD1357_NORMALDISPLAY = 0xA6;
static const uint8_t SSD1357_INVERTDISPLAY = 0xA7;
// contrast controls
static const uint8_t SSD1357_CONTRASTABC = 0xC1; //Different
static const uint8_t SSD1357_CONTRASTMASTER = 0xC7;
// memory functions
static const uint8_t SSD1357_WRITERAM = 0x5C;
static const uint8_t SSD1357_READRAM = 0x5D;
// other functions
static const uint8_t SSD1357_FUNCTIONSELECT = 0xAB;  //not there 0xE3 is nop
static const uint8_t SSD1357_DISPLAYENHANCE = 0xB2;  //not there 0xE3 is nop
static const uint8_t SSD1357_SETVSL = 0xB4; //not there 0xE3 is nop
static const uint8_t SSD1357_SETGPIO = 0xB5; //not there 0xE3 is nop
static const uint8_t SSD1357_PRECHARGE2 = 0xB6;
static const uint8_t SSD1357_SETGRAY = 0xB8;
static const uint8_t SSD1357_USELUT = 0xB9;
static const uint8_t SSD1357_MUXRATIO = 0xCA; //Different
static const uint8_t SSD1357_COMMANDLOCK = 0xFD; //Different
static const uint8_t SSD1357_HORIZSCROLL = 0x96;
static const uint8_t SSD1357_STOPSCROLL = 0x9E;
static const uint8_t SSD1357_STARTSCROLL = 0x9F;

void SSD1357::setup() {
  this->init_internal_(this->get_buffer_length_());

  this->command(SSD1357_COMMANDLOCK);
  this->data(0x12);
//  this->command(SSD1357_COMMANDLOCK);
//  this->data(0xB1);
  this->command(SSD1357_DISPLAYOFF);
  this->command(SSD1357_CLOCKDIV);
  this->data(0x20);  // not f1
  this->command(SSD1357_MUXRATIO);
  this->data(0x7F); // same as 127
  this->command(SSD1357_DISPLAYOFFSET);
  this->data(0x00);
//  this->command(SSD1357_SETGPIO);
//  this->data(0x00);
//  this->command(SSD1357_FUNCTIONSELECT);
//  this->data(0x01);  // internal (diode drop)
  this->command(SSD1357_PRECHARGE);
  this->data(0x84); //change from x32
  this->command(SSD1357_VCOMH);
  this->data(0x07); //change from 05
//  this->command(SSD1357_NORMALDISPLAY);
//  this->command(SSD1357_SETVSL);
//  this->data(0xA0);
//  this->data(0xB5);
//  this->data(0x55);
  this->command(SSD1357_PRECHARGE2);
  this->data(0x01);
  this->command(SSD1357_SETREMAP);
  this->data(0x60); // was 34
  this->data(0x00); // needs extra 00 byte  
  this->command(SSD1357_STARTLINE);
  this->data(0x00);
  this->command(SSD1357_CONTRASTABC);
  this->data(0x32); // was c8
  this->data(0x29); // was 80
  this->data(0x53); // was c8
  this->command(SSD1357_PRECHARGELEVEL); // new
  this->data(0x00); // new
  this->command(SSD1357_CONTRASTMASTER); // new
  this->data(0x0F); // new 
  set_brightness(this->brightness_);
  this->fill(Color::BLACK);  // clear display - ensures we do not see garbage at power-on
  this->display();           // ...write buffer, which actually clears the display's memory
  this->turn_on();           // display ON
}
void SSD1357::display() {
  this->command(SSD1357_SETCOLUMN);  // set column address
  this->data(0x20);                  // set column start address was 0
  this->data(0x5F);                  // set column end address was 7f
  this->command(SSD1357_SETROW);     // set row address
  this->data(0x00);                  // set row start address
  this->data(0x7F);                  // set last row
  this->command(SSD1357_WRITERAM);
  this->write_display_data();
}
void SSD1357::update() {
  this->do_update_();
  this->display();
}
void SSD1357::set_brightness(float brightness) {
  // validation
  if (brightness > 1) {
    this->brightness_ = 1.0;
  } else if (brightness < 0) {
    this->brightness_ = 0;
  } else {
    this->brightness_ = brightness;
  }
  if (!this->is_ready()) {
    return;  // Component is not yet setup skip the command
  }
  // now write the new brightness level to the display
  this->command(SSD1357_CONTRASTMASTER);
  this->data(int(SSD1357_MAX_CONTRAST * (this->brightness_)));
}
bool SSD1357::is_on() { return this->is_on_; }
void SSD1357::turn_on() {
  this->command(SSD1357_DISPLAYON);
  this->is_on_ = true;
}
void SSD1357::turn_off() {
  this->command(SSD1357_DISPLAYOFF);
  this->is_on_ = false;
}
int SSD1357::get_height_internal() {
  switch (this->model_) {
    case SSD1357_MODEL_128_64:
      return 64;
    default:
      return 0;
  }
}
int SSD1357::get_width_internal() {
  switch (this->model_) {
    case SSD1357_MODEL_128_64:
      return 128;
    default:
      return 0;
  }
}
size_t SSD1357::get_buffer_length_() {
  return size_t(this->get_width_internal()) * size_t(this->get_height_internal()) * size_t(SSD1357_BYTESPERPIXEL);
}
void HOT SSD1357::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0)
    return;
  const uint32_t color565 = display::ColorUtil::color_to_565(color);
  // where should the bits go in the big buffer array? math...
  uint16_t pos = (x + 0x20 + y * this->get_width_internal()) * SSD1357_BYTESPERPIXEL;
  this->buffer_[pos++] = (color565 >> 8) & 0xff;
  this->buffer_[pos] = color565 & 0xff;
}
void SSD1357::fill(Color color) {
  // If clipping is active, fall back to base implementation
  if (this->get_clipping().is_set()) {
    Display::fill(color);
    return;
  }

  const uint32_t color565 = display::ColorUtil::color_to_565(color);
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
    if (i & 1) {
      this->buffer_[i] = color565 & 0xff;
    } else {
      this->buffer_[i] = (color565 >> 8) & 0xff;
    }
  }
}
void SSD1357::init_reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
    delay(1);
    // Trigger Reset
    this->reset_pin_->digital_write(false);
    delay(10);
    // Wake up
    this->reset_pin_->digital_write(true);
  }
}
const char *SSD1357::model_str_() {
  switch (this->model_) {
    case SSD1357_MODEL_128_64:
      return "SSD1357 128X64";
    default:
      return "Unknown";
  }
}

}  // namespace esphome::ssd1357_base
