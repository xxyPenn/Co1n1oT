#include <stdint.h>
#include <stdio.h> // for size_t

#define BLACK 0
#define WHITE 1

#define SSD1306_I2C_ADDRESS   0x3C	// 011110+SA0+RW - 0x3C or 0x3D
// Address for 128x32 is 0x3C
// Address for 128x32 is 0x3D (default) or 0x3C (if SA0 is grounded)

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SSD1306_128_64  128x64 pixel display

    SSD1306_128_32  128x32 pixel display

    You also need to set the LCDWIDTH and LCDHEIGHT defines to an
    appropriate size

    -----------------------------------------------------------------------*/
   #define SSD1306_128_64
/*=========================================================================*/

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

typedef unsigned char byte;

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 32
#endif

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

#define FLAG_PAD_ZERO 1
#define FLAG_PIXEL_DOUBLE_H 2
#define FLAG_PIXEL_DOUBLE_V 4
#define FLAG_PIXEL_DOUBLE (FLAG_PIXEL_DOUBLE_H | FLAG_PIXEL_DOUBLE_V)

typedef enum {
	FONT_SIZE_SMALL = 0,
	FONT_SIZE_MEDIUM,
	FONT_SIZE_LARGE,
	FONT_SIZE_XLARGE
} FONT_SIZE;

uint8_t _i2caddr;
byte m_col;
byte m_row;
byte m_font;
byte m_flags;

void i2c_begin(void);

void i2c_invertDisplay(uint8_t i);
void i2c_draw8x8(byte* buffer, byte x, byte y);

void i2c_startscrollright(uint8_t start, uint8_t stop);
void i2c_startscrollleft(uint8_t start, uint8_t stop);

void i2c_startscrolldiagright(uint8_t start, uint8_t stop);
void i2c_startscrolldiagleft(uint8_t start, uint8_t stop);
void i2c_stopscroll(void);

void i2c_fill(unsigned char dat);
  
  typedef struct twi_packet_t {
	  uint8_t chip;        ///< Address of the I2C device
	//  const uint8_t *buffer;  ///< Pointer to array buffer that we will write from
	  uint8_t *buffer;         ///< Pointer to array buffer that we will get message to
	  uint32_t addr_length;         ///< Length of message to read/write;
	  uint32_t length;        ///< Length of message to read/write;

  } twi_packet_t;
  
  
void i2c_printInt(unsigned int value, char padding);
void i2c_printLong(unsigned long value, char padding);
void i2c_setCursor(byte column, byte line);
size_t i2c_write_char(uint8_t c);
void i2c_writeDigit(byte n);
void i2c_draw(const byte * buffer, byte x, byte y, byte width, byte height);
void i2c_clearLine(byte line);
void i2c_clear(void);
size_t i2c_print(const char * str);
