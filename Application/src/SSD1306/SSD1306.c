
#include "SSD1306.h"
#include "i2cDriver/I2cDriver.h"
#include "fonts.h"
//#include <asf.h>
#include <string.h>

//static void ssd1306_data(uint8_t c);
static void ssd1306_command(uint8_t c);
extern void mdelay(uint32_t ul_dly_ticks);


static uint32_t i2c_write(uint8_t * data, uint32_t len)
{	
	
	I2C_Data OLED;

	OLED.address = SSD1306_I2C_ADDRESS;
	OLED.msgOut = data;
	OLED.lenOut = len;
	int32_t error = I2cWriteDataWait(&OLED, portMAX_DELAY);
	
	if (error!= ERROR_NONE){
		
		return error;
	}else return 0;
	
	
//	return twi_master_write(TWI0, &packet_tx);
}

//////////////////////////////Arduino Wire to TWI bridge/////////////////////////////

static inline byte pgm_read_byte(const byte * ptr)
{
	return *ptr;
}

static byte _data[200];
static byte _length;

static void WirebeginTransmission(byte a)
{
	(void)a;
	_length = 0;
}

static void Wirewrite(byte data)
{
	_data[_length++] = data;
}

static void Wirewrite2(byte * data, byte len)
{
	for(int a = 0; a < len; ++a)
	{
		_data[_length++] = data[a];
	}
}

static void WireendTransmission(void)
{
	i2c_write(_data, _length);
}

/////////////////////////////////the rest of library/////////////////////////////

static void ssd1306_command(uint8_t c) {
	// I2C
	uint8_t control = 0x00;   // Co = 0, D/C = 0
	const uint8_t length = 2U;
	uint8_t data[length];
	data[0] = control;
	data[1] = c;
	i2c_write(data, length);
}

void i2c_begin() {
	uint8_t vccstate = SSD1306_SWITCHCAPVCC;
	uint8_t i2caddr = SSD1306_I2C_ADDRESS;
	m_font = FONT_SIZE_SMALL;
	m_flags = FLAG_PAD_ZERO;
	_i2caddr = i2caddr;
	
	//wait for older OLEDs to start up after power on
	delay_ms(100);
		
    #if defined SSD1306_128_32
    // Init sequence for 128x32 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x1F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x10); }
    else
    { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x22); }
    else
    { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
    #endif

    #if defined SSD1306_128_64
    // Init sequence for 128x64 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x3F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x10); }
    else
    { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x9F); }
    else
    { ssd1306_command(0xCF); }
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x22); }
    else
    { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
    #endif

    ssd1306_command(SSD1306_DISPLAYON);//--turn on oled panel

    // clear screen
    delay_ms(5);

    ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
    ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

    for (byte i = 0; i < SSD1306_LCDHEIGHT / 8; i++) {
	    // send a bunch of data in one xmission
	    ssd1306_command(0xB0 + i);//set page address
	    ssd1306_command(0);//set lower column address
	    ssd1306_command(0x10);//set higher column address

	    for(byte j = 0; j < 8; j++){
		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte k = 0; k < SSD1306_LCDWIDTH / 8; k++) {
			    Wirewrite(0);
		    }
		    WireendTransmission();
	    }
    }
	
	//set brightness to dim the display slightly if the display is too bright
	//ssd1306_command(SSD1306_SETPRECHARGE);
	//ssd1306_command(16);
	//ssd1306_command(SSD1306_SETCONTRAST);
	//ssd1306_command(255);
}


void i2c_invertDisplay(uint8_t i) {
  if (i) {
    ssd1306_command(SSD1306_INVERTDISPLAY);
  } else {
    ssd1306_command(SSD1306_NORMALDISPLAY);
  }
}

// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void i2c_startscrollright(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void i2c_startscrollleft(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void i2c_startscrolldiagright(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void i2c_startscrolldiagleft(uint8_t start, uint8_t stop){
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void i2c_stopscroll(void){
	ssd1306_command(SSD1306_DEACTIVATE_SCROLL);
}

//static void ssd1306_data(uint8_t c) {
    //// I2C
    //uint8_t control = 0x40;   // Co = 0, D/C = 1
	//const uint8_t length =2U;
	//uint8_t data[length];
	//data[0] = control;
	//data[1] = c;
	//i2c_write(data, length);
//}

void i2c_fill(unsigned char dat)
{
    ssd1306_command(0x00);//set lower column address
    ssd1306_command(0x10);//set higher column address
    ssd1306_command(0xB0);//set page address

    for (byte i=0; i<(SSD1306_LCDHEIGHT/8); i++)
    {
        // send a bunch of data in one xmission
        ssd1306_command(0xB0 + i);//set page address
        ssd1306_command(0);//set lower column address
        ssd1306_command(0x10);//set higher column address

        for(byte j = 0; j < 8; j++){
            WirebeginTransmission(_i2caddr);
            Wirewrite(0x40);
            for (byte k = 0; k < 16; k++) {
                Wirewrite(dat);
            }
            WireendTransmission();
        }
    }
}

void i2c_draw8x8(byte* buffer, uint8_t x, uint8_t y)
{
    // send a bunch of data in one xmission
    ssd1306_command(0xB0 + y);//set page address
    ssd1306_command(x & 0xf);//set lower column address
    ssd1306_command(0x10 | (x >> 4));//set higher column address

    WirebeginTransmission(_i2caddr);
    Wirewrite(0x40);
    Wirewrite2(buffer, 8);
    WireendTransmission();
}

void i2c_printInt(unsigned int value, char padding)
{
	unsigned int den = 10000;
	for (byte i = 5; i > 0; i--) {
		byte v = (byte)(value / den);
		value -= v * den;
		den /= 10;
		if (v == 0 && padding && den) {
			if (padding >= i) {
				i2c_writeDigit((m_flags & FLAG_PAD_ZERO) ? 0 : -1);
			}
			continue;
		}
		padding = 0;
		i2c_writeDigit(v);
	}
}

void i2c_printLong(unsigned long value, char padding)
{
	unsigned long den = 1000000000;
	for (byte i = 10; i > 0; i--) {
		byte v = (byte)(value / den);
		value -= v * den;
		den /= 10;
		if (v == 0 && padding && den) {
			if (padding >= i) {
				i2c_writeDigit((m_flags & FLAG_PAD_ZERO) ? 0 : -1);
			}
			continue;
		}
		padding = 0;
		i2c_writeDigit(v);
	}
}

void i2c_setCursor(byte column, byte line)
{
	m_col = column;
	m_row = line;
	ssd1306_command(0xB0 + m_row);//set page address
	ssd1306_command(m_col & 0xf);//set lower column address
	ssd1306_command(0x10 | (m_col >> 4));//set higher column address
}

size_t i2c_write_char(uint8_t c)
{
	
	if (c == '\n') {
		i2c_setCursor(0, m_row + ((m_font == FONT_SIZE_SMALL) ? 1 : 2));
		return 1;
		} else if (c == '\r') {
		m_col = 0;
		return 1;
	}
	#ifndef MEMORY_SAVING
	if (m_font == FONT_SIZE_SMALL) {
		#endif
		WirebeginTransmission(_i2caddr);
		Wirewrite(0x40);
		if (c > 0x20 && c < 0x7f) {
			c -= 0x21;
			for (byte i = 0; i < 5; i++) {
				byte d = pgm_read_byte(&font5x8[c][i]);
				Wirewrite(d);
				if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
			}
			Wirewrite(0);
			} else {
			for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 11 : 6; i > 0; i--) {
				Wirewrite(0);
			}
		}
		WireendTransmission();
		m_col += (m_flags & FLAG_PIXEL_DOUBLE_H) ? 11 : 6;
		if (m_col >= 128) {
			m_col = 0;
			m_row ++;
		}
		#ifndef MEMORY_SAVING
		} else {
		if (c > 0x20 && c < 0x7f) {
			c -= 0x21;

			ssd1306_command(0xB0 + m_row);//set page address
			ssd1306_command(m_col & 0xf);//set lower column address
			ssd1306_command(0x10 | (m_col >> 4));//set higher column address

			WirebeginTransmission(_i2caddr);
			Wirewrite(0x40);
			for (byte i = 0; i <= 14; i += 2) {
				byte d = pgm_read_byte(&font8x16_terminal[c][i]);
				Wirewrite(d);
				if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
			}
			WireendTransmission();

			ssd1306_command(0xB0 + m_row + 1);//set page address
			ssd1306_command(m_col & 0xf);//set lower column address
			ssd1306_command(0x10 | (m_col >> 4));//set higher column address

			WirebeginTransmission(_i2caddr);
			Wirewrite(0x40);
			for (byte i = 1; i <= 15; i += 2) {
				byte d = pgm_read_byte(&font8x16_terminal[c][i]);
				Wirewrite(d);
				if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
			}
			WireendTransmission();
			} else {
			ssd1306_command(0xB0 + m_row);//set page address
			ssd1306_command(m_col & 0xf);//set lower column address
			ssd1306_command(0x10 | (m_col >> 4));//set higher column address

			WirebeginTransmission(_i2caddr);
			Wirewrite(0x40);
			for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 16 : 8; i > 0; i--) {
				Wirewrite(0);
			}
			WireendTransmission();

			ssd1306_command(0xB0 + m_row + 1);//set page address
			ssd1306_command(m_col & 0xf);//set lower column address
			ssd1306_command(0x10 | (m_col >> 4));//set higher column address

			WirebeginTransmission(_i2caddr);
			Wirewrite(0x40);
			for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 16 : 8; i > 0; i--) {
				Wirewrite(0);
			}
			WireendTransmission();
		}
		m_col += (m_flags & FLAG_PIXEL_DOUBLE_H) ? 17 : 9;
		if (m_col >= 128) {
			m_col = 0;
			m_row += 2;
		}
	}
	#endif
	return 1;
}

void i2c_writeDigit(byte n)
{
    if (m_font == FONT_SIZE_SMALL) {
	    WirebeginTransmission(_i2caddr);
	    Wirewrite(0x40);
	    if (n <= 9) {
		    n += '0' - 0x21;
		    for (byte i = 0; i < 5; i++) {
			    Wirewrite(pgm_read_byte(&font5x8[n][i]));
		    }
		    Wirewrite(0);
		    } else {
		    for (byte i = 0; i < 6; i++) {
			    Wirewrite(0);
		    }
	    }
	    WireendTransmission();
	    m_col += 6;
	    } else if (m_font == FONT_SIZE_MEDIUM) {
	    i2c_write_char(n <= 9 ? ('0' + n) : ' ');
	    #ifndef MEMORY_SAVING
	    } else if (m_font == FONT_SIZE_LARGE) {
	    if (n <= 9) {
		    byte i;
		    ssd1306_command(0xB0 + m_row);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (i = 0; i < 16; i ++) {
			    byte d = pgm_read_byte(&digits16x16[n][i]);
			    Wirewrite(d);
			    if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 1);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (; i < 32; i ++) {
			    byte d = pgm_read_byte(&digits16x16[n][i]);
			    Wirewrite(d);
			    if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
		    }
		    WireendTransmission();
		    } else {
		    ssd1306_command(0xB0 + m_row);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 32 : 16; i > 0; i--) {
			    Wirewrite(0);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 1);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 32 : 16; i > 0; i--) {
			    Wirewrite(0);
		    }
		    WireendTransmission();
	    }
	    m_col += (m_flags & FLAG_PIXEL_DOUBLE_H) ? 30 : 16;
	    #endif
	    } else {
	    if (n <= 9) {
		    byte i;
		    ssd1306_command(0xB0 + m_row);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (i = 0; i < 16; i ++) {
			    byte d = pgm_read_byte(&digits16x24[n][i * 3]);
			    Wirewrite(d);
			    if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 1);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (i = 0; i < 16; i ++) {
			    byte d = pgm_read_byte(&digits16x24[n][i * 3 + 1]);
			    Wirewrite(d);
			    if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 2);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (i = 0; i < 16; i ++) {
			    byte d = pgm_read_byte(&digits16x24[n][i * 3 + 2]);
			    Wirewrite(d);
			    if (m_flags & FLAG_PIXEL_DOUBLE_H) Wirewrite(d);
		    }
		    WireendTransmission();
		    } else {
		    ssd1306_command(0xB0 + m_row);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 32 : 16; i > 0; i--) {
			    Wirewrite(0);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 1);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 32 : 16; i > 0; i--) {
			    Wirewrite(0);
		    }
		    WireendTransmission();

		    ssd1306_command(0xB0 + m_row + 2);//set page address
		    ssd1306_command(m_col & 0xf);//set lower column address
		    ssd1306_command(0x10 | (m_col >> 4));//set higher column address

		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte i = (m_flags & FLAG_PIXEL_DOUBLE_H) ? 32 : 16; i > 0; i--) {
			    Wirewrite(0);
		    }
		    WireendTransmission();
	    }
	    m_col += (m_flags & FLAG_PIXEL_DOUBLE_H) ? 30 : 16;
    }
}

void i2c_draw(const byte* buffer, byte x, byte y, byte width, byte height)
{
    ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
    ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

    const byte *p = buffer;
    height >>= 3;
    width >>= 3;
    y >>= 3;
    for (byte i = 0; i < height; i++) {
	    // send a bunch of data in one xmission
	    ssd1306_command(0xB0 + i + y);//set page address
	    ssd1306_command(x & 0xf);//set lower column address
	    ssd1306_command(0x10 | (x >> 4));//set higher column address

	    for(byte j = 0; j < 8; j++){
		    WirebeginTransmission(_i2caddr);
		    Wirewrite(0x40);
		    for (byte k = 0; k < width; k++, p++) {
			    Wirewrite(pgm_read_byte(p));
		    }
		    WireendTransmission();
	    }
    }
}

void i2c_clearLine(byte line)
{
	ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
	ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

	// send a bunch of data in one xmission
	ssd1306_command(0xB0 + line);//set page address
	ssd1306_command(0);//set lower column address
	ssd1306_command(0x10);//set higher column address

	for(byte j = 0; j < 8; j++){
		WirebeginTransmission(_i2caddr);
		Wirewrite(0x40);
		for (byte k = 0; k < 16; k++) {
			Wirewrite(0);
		}
		WireendTransmission();
	}
}

void i2c_clear()
{
	byte x = 0;
	byte y = 0;
	byte width = 128;
	byte height = 64;
	
	ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
	ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

	height >>= 3;
	width >>= 3;
	y >>= 3;
	for (byte i = 0; i < height; i++) {
		// send a bunch of data in one xmission
		ssd1306_command(0xB0 + i + y);//set page address
		ssd1306_command(x & 0xf);//set lower column address
		ssd1306_command(0x10 | (x >> 4));//set higher column address

		for(byte j = 0; j < 8; j++){
			WirebeginTransmission(_i2caddr);
			Wirewrite(0x40);
			for (byte k = 0; k < width; k++) {
				Wirewrite(0);
			}
			WireendTransmission();
		}
	}

	i2c_setCursor(0, 0);
}

size_t i2c_print(const char * str)
{
	if (str == NULL) 
	{
		return 0;
	}
	else
	{
		size_t size = strlen(str);
		size_t n = 0;
		while (size--) {
			if (i2c_write_char(*str++)) n++;
			else break;
		}
		return n;
	}
}