#include "main.h"
#include "st7793.h"
#include "font.h"

#define HIGH 1
#define LOW 0

static const char* TAG = "ST7793";

#define CMD_DELAY  0xFFFF

static const uint16_t st7793_initTab[] = {
        0x0008, 0x0808,
        0x0010, 0x0016,      //69.5Hz         0016
        0x0011, 0x0101,
        0x0012, 0x0000,
        0x0013, 0x0001,
        0x0100, 0x0330,      //BT,AP
        0x0101, 0x0237,      //DC0,DC1,VC
        0x0103, 0x0D00,      //VDV
        0x0280, 0x6100,      //VCM
        0x0102, 0xC1B0,      //VRH,VCMR,PSON,PON
        CMD_DELAY, 50,
        0x0001, 0x0000,
        0x0002, 0x0100,
// TRI[15]
// DFM[14]
// BGR[12]=0, 0 - RGB in network order, 1 - RGB in little-endian mode
// ORG[7]=1, 0 - origin is not moved, 1 - the original address 00000h moves according to the I/D[5..4] setting.
// I/D[5..4]=11, controlls update direction, see doc
// AM[3]=0, 0 - set the horizontal writing direction, 1 - set the vertical writing direction.
        0x0003, 0b0001000010100000,       // B0 Entry Mode (R003h), BGR=1, ORG=1, I/D = 10, AM=0
        0x0009, 0x0001,
        0x000C, 0x0000,
        0x0090, 0x8000,
        0x000F, 0x0000,
        0x0210, 0x0000,       // Window start X
        0x0211, 0x00EF,       // Window end X, EF=239, F9=249
        0x0212, 0x0000,       // Window start Y
        0x0213, 0x018F,       // Window end Y,  432=01AF,400=018F
        0x0500, 0x0000,
        0x0501, 0x0000,
        0x0502, 0x005F,
        0x0400, 0x6200,       //NL=0x31 (49) i.e. 400 rows for ST7793, GS=0
        0x0401, 0x0001,
        0x0404, 0x0000,
        CMD_DELAY, 50,
        0x0007, 0x0100,         //BASEE
        CMD_DELAY, 50,
        0x0200, 0x0000,         // X start address
        0x0201, 0x0000,         // Y start address
};

void ST7793Driver::init( uint16_t width, uint16_t height, int clkFreq,
        gpio_num_t _rd, gpio_num_t _wr, gpio_num_t _rs, gpio_num_t _cs, gpio_num_t _d0, gpio_num_t _d1,
        gpio_num_t _d2, gpio_num_t _d3, gpio_num_t _d4, gpio_num_t _d5, gpio_num_t _d6, gpio_num_t _d7,
        gpio_num_t _rst )
{
    m_width = width;
    m_height = height;

    m_RD = _rd;
    m_WR = _wr;
    m_RS = _rs;
    m_CS = _cs;
    m_D0 = _d0;
    m_D1 = _d1;
    m_D2 = _d2;
    m_D3 = _d3;
    m_D4 = _d4;
    m_D5 = _d5;
    m_D6 = _d6;
    m_D7 = _d7;
    m_RST = _rst;

    //ESP_LOGI( TAG, "CS = %d", m_CS );
    gpio_reset_pin( m_CS );
    gpio_set_direction( m_CS, GPIO_MODE_OUTPUT );
    gpio_set_level( m_CS, HIGH );

    //ESP_LOGI( TAG, "RS = %d", m_RS );
    gpio_reset_pin( m_RS );
    gpio_set_direction( m_RS, GPIO_MODE_OUTPUT );
    gpio_set_level( m_RS, HIGH );

    //ESP_LOGI( TAG, "WR = %d", m_WR );
    gpio_reset_pin( m_WR );
    gpio_set_direction( m_WR, GPIO_MODE_OUTPUT );
    gpio_set_level( m_WR, HIGH );

    //ESP_LOGI( TAG, "RD = %d", m_RD );
    gpio_reset_pin( m_RD );
    gpio_set_direction( m_RD, GPIO_MODE_OUTPUT );
    gpio_set_level( m_RD, HIGH );

    //ESP_LOGI( TAG, "D0 = %d", m_D0 );
    //ESP_LOGI( TAG, "D1 = %d", m_D1 );
    //ESP_LOGI( TAG, "D2 = %d", m_D2 );
    //ESP_LOGI( TAG, "D3 = %d", m_D3 );
    //ESP_LOGI( TAG, "D4 = %d", m_D4 );
    //ESP_LOGI( TAG, "D5 = %d", m_D5 );
    //ESP_LOGI( TAG, "D6 = %d", m_D6 );
    //ESP_LOGI( TAG, "D7 = %d", m_D7 );

    m_pixelsCount  = m_width * m_height;
    m_frameBufSize = m_pixelsCount * sizeof(uint16_t);

    m_pFrameBuffer = (uint16_t*)heap_caps_malloc( m_frameBufSize, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM );
    if( !m_pFrameBuffer ) {
        ESP_LOGE( TAG, "Can't allocate frame buffer!" );
        assert( 0 );
    }

    //ESP_LOGI( TAG, "WIDTH: %u, HEIGH: %u,\nAllocated frame buffer %lu bytes", m_width, m_height, m_frameBufSize );

    //ESP_LOGI( TAG, "Initializing I2S" );
    i2s_lcd_config_t i2s_lcd_cfg = {
        .data_width  = LCD_I2S_BIT_WIDTH,
        .pin_data_num = {
            m_D0,
            m_D1,
            m_D2,
            m_D3,
            m_D4,
            m_D5,
            m_D6,
            m_D7,
        },
        .pin_num_cs  = m_CS,
        .pin_num_wr  = m_WR,
        .pin_num_rs  = m_RS,
        .clk_freq    = clkFreq,
        .i2s_port    = I2S_NUM_0,
        .swap_data   = true,    // DMA will swap bytes to network order
        .buffer_size = 16000    // DMA buffer size
    };

    m_hI2S = i2s_lcd_driver_init( &i2s_lcd_cfg );
    if( !m_hI2S )
    {
        ESP_LOGE( TAG, "%s:%d (%s): 8080 interface init failed", __FILE__, __LINE__, __FUNCTION__ );
        assert( 0 );
    }

    //ESP_LOGI( TAG, "reset LCD (%d) pin", m_RST );
    gpio_reset_pin( m_RST );
    gpio_set_direction( m_RST, GPIO_MODE_OUTPUT );
    gpio_set_level( m_RST, HIGH );
    vTaskDelay( pdMS_TO_TICKS( 100 ) );
    gpio_set_level( m_RST, LOW );
    vTaskDelay( pdMS_TO_TICKS( 100 ) );
    gpio_set_level( m_RST, HIGH );
    vTaskDelay( pdMS_TO_TICKS( 100 ) );

    // send initialization commands to lcd
    init_table( st7793_initTab, sizeof(st7793_initTab) / sizeof(uint16_t) );
}

void ST7793Driver::init_table( const uint16_t* table, int16_t length )
{
    int numElements = length / 2;
    while( numElements-- > 0 )
    {
        uint16_t cmd = *table++;
        uint16_t dat = *table++;

        if( cmd == CMD_DELAY ) { // apply small delay
            vTaskDelay( pdMS_TO_TICKS( dat ) );
        } else { // write command to display
            //ESP_LOGI( TAG, "sending cmd: 0x%04X, data: 0x%04X", cmd, dat );
            write_reg( cmd, dat );
        }
    }
}

void ST7793Driver::write_reg( uint16_t addr, uint16_t data )
{
    write_cmd( addr );
    write_data( data);
}

void ST7793Driver::write_cmd( uint16_t cmd )
{
    gpio_set_level( m_CS, LOW );
    gpio_set_level( m_RS, LOW );

    i2s_lcd_write( m_hI2S, (uint8_t*)&cmd, sizeof(uint16_t) );

    gpio_set_level( m_CS, HIGH );
}

void ST7793Driver::write_data( uint16_t data )
{
    gpio_set_level( m_CS, LOW );
    gpio_set_level( m_RS, HIGH );

    i2s_lcd_write( m_hI2S, (uint8_t*)&data, sizeof(uint16_t) );

    gpio_set_level( m_CS, HIGH );
}

void ST7793Driver::rotate( uint8_t rot )
{
    uint16_t REG_0400 = 0, REG_0001 = 0, REG_0003 = 0;

    // make sure that w > h for simplicity
    uint16_t w = m_width  > m_height ? m_width : m_height;
    uint16_t h = m_height > m_width  ? m_width : m_height;

    switch( rot & 3 )
    {
    case 0:        // PORTRAIT
        REG_0400 = 0x6200;
        REG_0001 = 0x0100;
        REG_0003 = 0x1030;
        m_width  = h;
        m_height = w;
        break;
    case 1:       // LANDSCAPE
        REG_0400 = 0x6200;
        REG_0001 = 0x0000;
        REG_0003 = 0x1038;
        m_width  = w;
        m_height = h;
        break;
    case 2:       // PORTRAIT revresed
        REG_0400 = 0xE200;
        REG_0001 = 0x0000;
        REG_0003 = 0x1030;
        m_width  = h;
        m_height = w;
        break;
    case 3:      // LANDSCAPE reversed
        REG_0400 = 0xE200;
        REG_0001 = 0x0100;
        REG_0003 = 0x1038;
        m_width  = w;
        m_height = h;
        break;
    }

    write_reg( 0x0400, REG_0400 );
    write_reg( 0x0001, REG_0001 );
    write_reg( 0x0003, REG_0003 );
}

// Fills back buffer with specified color
void ST7793Driver::clearFrameBuffer( uint16_t color )
{
    uint32_t len = m_pixelsCount;
    uint16_t* pFrameBuf = m_pFrameBuffer;

    while( len-- > 0 )
        *pFrameBuf++ = color;
}

// Draws Vertical line into frame buffer
void ST7793Driver::drawVLine( uint16_t x, uint16_t y1, uint16_t y2, uint16_t color )
{
    if( x > m_width )
        x = m_width;
    if( y1 > m_height )
        y1 = m_height;
    if( y2 > m_height )
        y2 = m_height;
    if( y1 > y2 ) {
        uint16_t tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    uint16_t* pFrameBuf = &m_pFrameBuffer[0];

    for( uint16_t y = y1; y < y2; y++ ) {
        pFrameBuf[ y * m_width + x ] = color;
    }
}

// Draws Horisontal line into frame buffer
void ST7793Driver::drawHLine( uint16_t x1, uint16_t x2, uint16_t y, uint16_t color )
{
    if( y > m_height )
        y = m_height;
    if( x1 > m_width )
        x1 = m_width;
    if( x2 > m_width )
        x2 = m_width;
    if( x1 > x2 ) {
        uint16_t tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    uint16_t* pixel = &( m_pFrameBuffer[ y * m_width + x1 ] );

    for( uint16_t x = x1; x < x2; x++ ) {
        *pixel++ = color;
    }    
}

// Transfer content of the frame buffer to display
void ST7793Driver::flip()
{
    write_reg( 0x0200, 0 );     // X start address
    write_reg( 0x0201, 0 );     // Y start address
    write_cmd( 0x0202 );        // write data to DRAM

    gpio_set_level( m_CS, LOW );
    gpio_set_level( m_RS, HIGH );

    // send entire frame buffer to display
    i2s_lcd_write( m_hI2S, (uint8_t*)m_pFrameBuffer, m_frameBufSize );
    
    gpio_set_level( m_CS, HIGH );
}

void ST7793Driver::drawLine( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color )
{
    // distance between two points
    int dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
    int dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

    // direction of two points
    int sx = ( x2 > x1 ) ? 1 : -1;
    int sy = ( y2 > y1 ) ? 1 : -1;

    if ( dx > dy ) { // inclination < 1
        int E = -dx;
        for( int i = 0 ; i <= dx ; i++ ) {
            put_pixel( x1, y1, color );
            x1 += sx;
            E += 2 * dy;
            if ( E >= 0 ) {
                y1 += sy;
                E -= 2 * dx;
            }
        }
    } else { // inclination >= 1
        int E = -dy;
        for( int i = 0 ; i <= dy ; i++ ) {
            put_pixel( x1, y1, color );
            y1 += sy;
            E += 2 * dx;
            if ( E >= 0 ) {
                x1 += sx;
                E -= 2 * dy;
            }
        }
    }
}

static const BYTE bitFlags[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

void ST7793Driver::putChar( char ch, uint16_t x, uint16_t y, uint16_t color )
{
    const FONT& font = getSystemFont();
    const GLYPH& glyph = font.Glyphs[ch-32];

    for( uint16_t i = 0; i < glyph.wHeight; i++ )
    {
        byte b = glyph.g[i];
        
        for( uint16_t k = 0, j = 7; k < 8; k++, j-- ) // output pixels bit by bit
        {
            if( b & bitFlags[j] )
                put_pixel( x+k, y+i, color );
        }
    }
}

void ST7793Driver::printText( lpcstr text, uint16_t x, uint16_t y, uint16_t color )
{
    lpcstr p = text;
    const FONT& font = getSystemFont();
    
    while( *p )
    {
        const GLYPH& glyph = font.Glyphs[*p-32];
      
        for( uint16_t i = 0; i < glyph.wHeight; i++ )
        {
            byte c = glyph.g[i];
            
            for( uint16_t k = 0, j = 7; k < 8; k++, j-- ) // output pixels bit by bit
            {
                if( c & bitFlags[j] )
                    put_pixel( x+k, y+i, color );
            }
        }

        x += glyph.wWidth;
        p++;
    }
}
