#pragma once

#include "i2s_lcd_driver.h"

#define LCD_I2S_BIT_WIDTH 8

#define LCD_WIDTH 240
#define LCD_HEIGHT 400

class ST7793Driver
{
    uint16_t m_width;
    uint16_t m_height;

    gpio_num_t m_RD;
    gpio_num_t m_WR;
    gpio_num_t m_RS;
    gpio_num_t m_CS;
    gpio_num_t m_D0;
    gpio_num_t m_D1;
    gpio_num_t m_D2;
    gpio_num_t m_D3;
    gpio_num_t m_D4;
    gpio_num_t m_D5;
    gpio_num_t m_D6;
    gpio_num_t m_D7;
    gpio_num_t m_RST;

    uint16_t* m_pFrameBuffer; // X_RES * Y_RES * sizeof(unit16_t)
    uint32_t  m_frameBufSize; // in bytes!
    uint32_t  m_pixelsCount;  // size in pixels

    i2s_lcd_handle_t m_hI2S;

    void write_reg( uint16_t addr, uint16_t data );
    void write_cmd( uint16_t cmd );
    void write_data( uint16_t data );
    void init_table( const uint16_t* table, int16_t length ); // length in elements

    inline void put_pixel( uint16_t x, uint16_t y, uint16_t c ) {
        m_pFrameBuffer[ y * m_width + x ] = c;
    }

public:
    void init( uint16_t width, uint16_t height, int clkFreq,
        gpio_num_t _rd, gpio_num_t _wr, gpio_num_t _rs, gpio_num_t _cs, gpio_num_t _d0, gpio_num_t _d1,
        gpio_num_t _d2, gpio_num_t _d3, gpio_num_t _d4, gpio_num_t _d5, gpio_num_t _d6, gpio_num_t _d7,
        gpio_num_t _rst );

    inline uint16_t width() const {
        return m_width;
    }

    inline uint16_t height() const {
        return m_height;
    }

    inline uint16_t* getFrameBuffer() const {
        return m_pFrameBuffer;
    }

    // Rotates display to landscape/portrait modes...
    void rotate( uint8_t rot );

    // Fills back buffer with specified color
    void clearFrameBuffer( uint16_t color );

    // Draws Vertical line into frame buffer
    void drawVLine( uint16_t x, uint16_t y1, uint16_t y2, uint16_t color );

    // Draws Horisontal line into frame buffer
    void drawHLine( uint16_t x1, uint16_t x2, uint16_t y, uint16_t color );

    // Draws an arbitrary line
    void drawLine( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color );

    // prints character into frame buffer
    void putChar( char ch, uint16_t x, uint16_t y, uint16_t color );

    // prints text into frame buffer
    void printText( lpcstr text, uint16_t x, uint16_t y, uint16_t color );

    // Transfer content of the frame buffer to display
    void flip();
};