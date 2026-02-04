/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    appglcd.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "appglcd.h"
#include "definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define LCD_START_LINE_ADDR	(66-2)
#define LCD_CONTRAST        0x60
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APPGLCD_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APPGLCD_DATA appglcdData;
const uint8_t commandsInitializeGLCD[0x08] = 
{
    0x21, // 0 LCD Extended Commands.
    0xC8, // 1 Set LCD Vop (Contrast). 0xC8
    0x04 | !!(LCD_START_LINE_ADDR & (1u << 6)),  // 2 Set Temp S6 for start line
    0x40 | (LCD_START_LINE_ADDR & ((1u << 6) - 1)), // 3 Set Temp S[5:0] for start line
    0x12, // 4 LCD bias mode 1:68.
    0x20, // 5 LCD Standard Commands, Horizontal addressing mode.
    0x08, // 6 LCD blank
    0x0C  // 7 LCD in normal mode
};
// (84*84)/8 = 882 bytes
const uint8_t logoSicoy[] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xcf, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xcf, 0xfc, 0x03, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0xcf, 0x80, 0x07, 0xfc, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xce, 0x00, 0x0f, 0xfc, 
	0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xcc, 0x03, 0x8e, 0x00, 0x00, 0xf0, 0x1e, 0x10, 0x20, 
	0x00, 0x01, 0xcc, 0xff, 0x8c, 0x00, 0x01, 0xfc, 0x7e, 0x18, 0x70, 0x00, 0x01, 0xcc, 0xf7, 0x8f, 
	0x00, 0xe3, 0xfe, 0xff, 0x18, 0x70, 0x00, 0x00, 0x0c, 0xc0, 0x0f, 0xfc, 0xe7, 0x8c, 0xe3, 0x98, 
	0x70, 0x00, 0x00, 0x0c, 0xc0, 0x07, 0xfe, 0xe7, 0x01, 0xc1, 0x98, 0x70, 0x00, 0x0f, 0x1c, 0xc4, 
	0x00, 0x1e, 0xe7, 0x01, 0xc1, 0x98, 0x70, 0x00, 0x0f, 0xfc, 0xc4, 0x00, 0x0e, 0xe7, 0x01, 0xc1, 
	0x98, 0x70, 0x00, 0x0f, 0x00, 0xc4, 0x00, 0x0e, 0xe7, 0x09, 0xe1, 0x9c, 0x70, 0x00, 0x04, 0x01, 
	0xc4, 0x07, 0xfe, 0xe7, 0xdc, 0xf7, 0x9e, 0xf0, 0x00, 0x00, 0x07, 0xc4, 0x07, 0xfc, 0xe3, 0xfc, 
	0x7f, 0x0f, 0xf0, 0x00, 0x01, 0xff, 0xc4, 0x0f, 0xf8, 0xe1, 0xf8, 0x3e, 0x07, 0xf0, 0x00, 0x01, 
	0xff, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t FontLookup [][5] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00}, // sp
    { 0x00, 0x00, 0x2f, 0x00, 0x00}, // !
    { 0x00, 0x07, 0x00, 0x07, 0x00}, // "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    { 0xc4, 0xc8, 0x10, 0x26, 0x46}, // %
    { 0x36, 0x49, 0x55, 0x22, 0x50}, // &
    { 0x00, 0x05, 0x03, 0x00, 0x00}, // '
    { 0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    { 0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    { 0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    { 0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    { 0x00, 0x00, 0x50, 0x30, 0x00}, // ,
    { 0x10, 0x10, 0x10, 0x10, 0x10}, // -
    { 0x00, 0x60, 0x60, 0x00, 0x00}, // .
    { 0x20, 0x10, 0x08, 0x04, 0x02}, // /
    { 0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    { 0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    { 0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    { 0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    { 0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00}, // :
    { 0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    { 0x08, 0x14, 0x22, 0x41, 0x00}, // <
    { 0x14, 0x14, 0x14, 0x14, 0x14}, // =
    { 0x00, 0x41, 0x22, 0x14, 0x08}, // >
    { 0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    { 0x32, 0x49, 0x59, 0x51, 0x3E}, // @
    { 0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    { 0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    { 0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    { 0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    { 0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    { 0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    { 0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    { 0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    { 0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    { 0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    { 0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    { 0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    { 0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    { 0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    { 0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    { 0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    { 0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    { 0x46, 0x49, 0x49, 0x49, 0x31}, // S
    { 0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    { 0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    { 0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    { 0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    { 0x63, 0x14, 0x08, 0x14, 0x63}, // X
    { 0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    { 0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    { 0x55, 0x2A, 0x55, 0x2A, 0x55}, // 55
    { 0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40}, // _
    { 0x00, 0x01, 0x02, 0x04, 0x00}, // '
    { 0x20, 0x54, 0x54, 0x54, 0x78}, // a
    { 0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    { 0x38, 0x44, 0x44, 0x44, 0x20}, // c
    { 0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    { 0x38, 0x54, 0x54, 0x54, 0x18}, // e
    { 0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    { 0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    { 0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    { 0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    { 0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    { 0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    { 0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    { 0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    { 0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    { 0x38, 0x44, 0x44, 0x44, 0x38}, // o
    { 0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    { 0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    { 0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    { 0x48, 0x54, 0x54, 0x54, 0x20}, // s
    { 0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    { 0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    { 0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    { 0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    { 0x44, 0x28, 0x10, 0x28, 0x44}, // x
    { 0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    { 0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    { 0x08, 0x6C, 0x6A, 0x19, 0x08}, // { (lighting)
    { 0x0C, 0x12, 0x24, 0x12, 0x0C}, // | (heart)
    { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E}, // square
};

const uint8_t TinyFont[95][3] = 
{
    {0x00, 0x00, 0x00}, // ' '
    {0x00, 0x17, 0x00}, // '!'
    {0x03, 0x00, 0x03}, // '"'
    {0x1F, 0x0A, 0x1F}, // '#'
    {0x0A, 0x1F, 0x05}, // '$'
    {0x13, 0x04, 0x19}, // '%'
    {0x0A, 0x15, 0x1A}, // '&'
    {0x00, 0x03, 0x00}, // '''
    {0x0E, 0x11, 0x00}, // '('
    {0x00, 0x11, 0x0E}, // ')'
    {0x05, 0x02, 0x05}, // '*'
    {0x04, 0x0E, 0x04}, // '+'
    {0x10, 0x08, 0x00}, // ','
    {0x04, 0x04, 0x04}, // '-'
    {0x00, 0x10, 0x00}, // '.'
    {0x18, 0x04, 0x03}, // '/'
    {0x1F, 0x11, 0x1F}, // '0'
    {0x00, 0x1F, 0x00}, // '1'
    {0x1D, 0x15, 0x17}, // '2'
    {0x15, 0x15, 0x1F}, // '3'
    {0x07, 0x04, 0x1F}, // '4'
    {0x17, 0x15, 0x1D}, // '5'
    {0x1F, 0x15, 0x1D}, // '6'
    {0x01, 0x01, 0x1F}, // '7'
    {0x1F, 0x15, 0x1F}, // '8'
    {0x17, 0x15, 0x1F}, // '9'
    {0x00, 0x0A, 0x00}, // ':'
    {0x10, 0x0A, 0x00}, // ';'
    {0x04, 0x0A, 0x11}, // '<'
    {0x0A, 0x0A, 0x0A}, // '='
    {0x11, 0x0A, 0x04}, // '>'
    {0x01, 0x15, 0x07}, // '?'
    {0x0E, 0x15, 0x16}, // '@'
    {0x1F, 0x05, 0x1F}, // 'A'
    {0x1F, 0x15, 0x0A}, // 'B'
    {0x0E, 0x11, 0x11}, // 'C'
    {0x1F, 0x11, 0x0E}, // 'D'
    {0x1F, 0x15, 0x11}, // 'E'
    {0x1F, 0x05, 0x01}, // 'F'
    {0x0E, 0x11, 0x1D}, // 'G'
    {0x1F, 0x04, 0x1F}, // 'H'
    {0x11, 0x1F, 0x11}, // 'I'
    {0x08, 0x10, 0x0F}, // 'J'
    {0x1F, 0x04, 0x1B}, // 'K'
    {0x1F, 0x10, 0x10}, // 'L'
    {0x1F, 0x06, 0x1F}, // 'M'
    {0x1F, 0x0E, 0x1F}, // 'N'
    {0x0E, 0x11, 0x0E}, // 'O'
    {0x1F, 0x05, 0x02}, // 'P'
    {0x0E, 0x19, 0x1E}, // 'Q'
    {0x1F, 0x0D, 0x16}, // 'R'
    {0x12, 0x15, 0x09}, // 'S'
    {0x01, 0x1F, 0x01}, // 'T'
    {0x0F, 0x10, 0x1F}, // 'U'
    {0x07, 0x18, 0x07}, // 'V'
    {0x1F, 0x0C, 0x1F}, // 'W'
    {0x1B, 0x04, 0x1B}, // 'X'
    {0x03, 0x1C, 0x03}, // 'Y'
    {0x19, 0x15, 0x13}, // 'Z'
    {0x1F, 0x11, 0x00}, // '['
    {0x03, 0x04, 0x18}, // '\'
    {0x00, 0x11, 0x1F}, // ']'
    {0x02, 0x01, 0x02}, // '^'
    {0x10, 0x10, 0x10}, // '_'
    {0x01, 0x02, 0x00}, // '`'
    {0x1A, 0x16, 0x1C}, // 'a'
    {0x1F, 0x14, 0x08}, // 'b'
    {0x0C, 0x12, 0x12}, // 'c'
    {0x08, 0x14, 0x1F}, // 'd'
    {0x0C, 0x1A, 0x14}, // 'e'
    {0x04, 0x1E, 0x05}, // 'f'
    {0x04, 0x2A, 0x1E}, // 'g'
    {0x1F, 0x04, 0x18}, // 'h'
    {0x00, 0x1D, 0x00}, // 'i'
    {0x10, 0x20, 0x1D}, // 'j'
    {0x1F, 0x08, 0x14}, // 'k'
    {0x11, 0x1F, 0x10}, // 'l'
    {0x1E, 0x0C, 0x1E}, // 'm'
    {0x1E, 0x02, 0x1C}, // 'n'
    {0x0C, 0x12, 0x0C}, // 'o'
    {0x3E, 0x0A, 0x04}, // 'p'
    {0x04, 0x0A, 0x3E}, // 'q'
    {0x1C, 0x02, 0x02}, // 'r'
    {0x14, 0x1E, 0x0A}, // 's'
    {0x02, 0x1F, 0x12}, // 't'
    {0x0E, 0x10, 0x1E}, // 'u'
    {0x06, 0x18, 0x06}, // 'v'
    {0x1E, 0x0C, 0x1E}, // 'w'
    {0x12, 0x0C, 0x12}, // 'x'
    {0x02, 0x24, 0x1E}, // 'y'
    {0x1A, 0x1E, 0x16}, // 'z'
    {0x04, 0x1B, 0x11}, // '{'
    {0x00, 0x1F, 0x00}, // '|'
    {0x11, 0x1B, 0x04}, // '}'
    {0x02, 0x01, 0x02}, // '~'
};

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
extern uint32_t abs_diff_uint32(uint32_t a, uint32_t b);
extern bool IsSPI1TaskIdle (void);
extern void LCDSend(unsigned char data, unsigned char cd);
/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
/* TODO:  Add any necessary local functions.
*/
bool IsGLCDTaskIdle (void);
void LCDClear(void);
void LCDUpdate(void);
void LCDPixelXY(uint32_t x, uint32_t y);
void LCDLine (int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void LCDStr(uint8_t row, const uint8_t *dataPtr, bool inv, bool updateLCD);
void LCDChrXY_Scaled(uint8_t x, uint8_t y, const uint8_t *dataPtr, uint8_t scale, bool updateLCD);
void drawInitialLogo(void);
void SetPixel(uint8_t x, uint8_t y);
void ClearPixel(uint8_t x, uint8_t y);
void updateLCDOrIdleState(void);
void LCDTinyStr(uint8_t x, uint8_t y, const char *dataPtr, bool updateLCD);
/****************************************************************************/
/*  It is used to clear the screen before drawing something.                */                                             
/*  Function : LCDClear                                                     */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDClear (void)
{
    appglcdData.stateToReturn = APPGLCD_STATE_IDLE; //And after the update, it should go into idel state until another task asks it to plot something.
    appglcdData.state = APPGLCD_STATE_START_CLEANING_GLCD;
}
/****************************************************************************/
/*  It is used to update the screen before drawing something.               */                                             
/*  Function : LCDUpdaate                                                   */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDUpdate(void)
{
    appglcdData.pointerY1 = 0x00;
    appglcdData.updateLCD = false;
    appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
    appglcdData.stateToReturn = APPGLCD_STATE_IDLE;
}
/****************************************************************************/
/*  Determines whether the GLCD module task is idle so it can send or receive 
 *  data */                                                             
/*  Function : IsGLCDTaskIdle                                               */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  true or false                                                */
/****************************************************************************/
bool IsGLCDTaskIdle (void)
{
    if (APPGLCD_STATE_IDLE == appglcdData.state)
    {
        return true;
    }
    return false;
}
/****************************************************************************/
/*  Send to LCD                                                             */
/*  Function : LCDPixelXY                                                   */
/*  Parameters                                                              */
/*  Input   :  x and y coordinates where you want to graph a point          */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDPixelXY(uint32_t x, uint32_t y)
{
    uint32_t index = 0;
    //uint16_t i = 0;
    
    // check for out off range
    if ((x > LCD_X_RES)||(x < 0)) return;
    if ((y > LCD_Y_RES)||(y < 0)) return;

    index = x + ((y/8))*84 ;
    appglcdData.LcdMemory[index] |= (uint8_t)(1<<(y%8));
    //The process of updating GLCD information begins
    //appglcdData.pointerY1 = 0x00;
    //appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
}
/****************************************************************************/
/*  Send to LCD                                                             */
/*  Function : LCDLine                                                      */
/*  Parameters                                                              */
/*  Input   :  x1, x2 and y1, y2 coordinates where you want to graph a line */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDLine (int32_t x1, int32_t y1, int32_t x2, int32_t y2) //draw a line
{      
    appglcdData.dx = abs (x2 - x1);
    appglcdData.dy = abs (y2 - y1);
    if ( x1 < x2) 
    {
        appglcdData.sx = 1;
    }
    else 
    {    
        appglcdData.sx = -1;
    }
    if (y1 < y2) 
    {
        appglcdData.sy = 1;
    }
    else 
    {    
        appglcdData.sy = -1;
    }
    
    appglcdData.error1 = appglcdData.dx - appglcdData.dy;
    appglcdData.pointerX1 = x1;
    appglcdData.pointerY1 = y1;
    appglcdData.pointerX2 = x2;
    appglcdData.pointerY2 = y2;
    appglcdData.state = APPGLCD_STATE_GRAPH_LINE;
}
/****************************************************************************/
/*  Send to LCD                                                             */
/*  Function : LCDStr                                                       */
/*  Parameters                                                              */
/*  Input   :  row in which you want to write the text dataPtr, if it is    */
/* inverted, inv must be different from zero                                */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDStr(uint8_t row, const uint8_t *dataPtr, bool inv, bool updateLCD)
{
    appglcdData.xMessage = 0x00;
    appglcdData.row = (uint16_t)row;
    appglcdData.dataPtr = (uint8_t*)dataPtr;
    appglcdData.inv = inv;
    appglcdData.updateLCD = updateLCD;
    appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_ROW;
}
/****************************************************************************/
/*  Plots an enlarged text in the X and Y coordinates.                      */
/*  Function : LCDChrXY_Scaled                                              */
/*  Parameters                                                              */
/*  Input   :  row in which you want to write the text dataPtr, if it is    */
/* inverted, inv must be different from zero                                */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDChrXY_Scaled(uint8_t x, uint8_t y, const uint8_t *dataPtr, uint8_t scale, bool updateLCD)
{
    appglcdData.pointerX1 = x;
    appglcdData.pointerY1 = y;
    appglcdData.dataPtr = (uint8_t*)dataPtr;
    appglcdData.updateLCD = updateLCD;
    appglcdData.scale = scale;
    appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_SCALED;
}
/****************************************************************************/
/*  Clear an enlarged text in the X and Y coordinates.                      */
/*  Function : LCDChrXY_Scaled                                              */
/*  Parameters                                                              */
/*  Input   :  row in which you want to write the text dataPtr, if it is    */
/* inverted, inv must be different from zero                                */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void LCDStr_Scaled_Clear(uint8_t x, uint8_t y, uint8_t len, uint8_t scale, bool updateLCD)
{
    appglcdData.pointerX1 = x;
    appglcdData.pointerY1 = y;
    appglcdData.pointerX2 = len;
    appglcdData.bit = 0x00;
    appglcdData.updateLCD = updateLCD;
    appglcdData.scale = scale;
    appglcdData.dx = 0x00;
    appglcdData.dy = 0x00;
    appglcdData.state = APPGLCD_STATE_START_CLEAR_MESSAGE_SCALED;
}

/****************************************************************************/
/*  Graphically represents Sicoy's initial logo.                                                            */
/*  Function : drawInitialLogo                                              */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void drawInitialLogo(void)
{
    appglcdData.pointerX1 = 0;
    appglcdData.pointerY1 = 0;
    appglcdData.col = 0;
    appglcdData.row = 0;
    appglcdData.logoSample = logoSicoy[0];
    appglcdData.state = APPGLCD_STATE_START_DRAW_LOGO;
}
/****************************************************************************/
/*  Plot a point at the x and y coordinates                                 */
/*  Function : drawInitialLogo                                              */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void SetPixel(uint8_t x, uint8_t y)
{
    if (x >= LCD_X_RES || y >= LCD_Y_RES) 
    {
        return;
    }
    uint16_t index = x + (y / 8) * LCD_X_RES;
    appglcdData.LcdMemory[index] |= (1 << (y % 8));
}
/****************************************************************************/
/*  Clear a point at the x and y coordinates                                 */
/*  Function : drawInitialLogo                                              */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void ClearPixel(uint8_t x, uint8_t y)
{
    if (x >= LCD_X_RES || y >= LCD_Y_RES) return;
    uint16_t index = x + (y / 8) * LCD_X_RES;
    appglcdData.LcdMemory[index] &= ~(1 << (y % 8));
}
/****************************************************************************/
/*  After modifying the arrangement for the LCD, you can transfer that      */
/* information to the screen or the task to go idle state.                  */
/*  Function : updateLCDOrIdleState                                         */
/*  Parameters                                                              */
/*  Input   :  Nothing                                                      */
/*  Output  :  Nothing                                                      */
/****************************************************************************/
void updateLCDOrIdleState(void)
{
    if (appglcdData.updateLCD)
    {
        appglcdData.updateLCD = false;
        appglcdData.pointerY1 = 0x00; //This variable must be zero before clearing
        appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
        appglcdData.stateToReturn = APPGLCD_STATE_IDLE;
    }
    else
    {
        appglcdData.state = APPGLCD_STATE_IDLE;
    }
}
/****************************************************************************/
/*  Write an small text in the X and Y coordinates.                         */
/*  Function : LCDTinyStr                                                   */
/*  Parameters                                                              */
/*  Input   :  row in which you want to write the text dataPtr, if it is    */
/* inverted, inv must be different from zero                                */
/*  Output  :  Nothing                                                      */
void LCDTinyStr(uint8_t x, uint8_t y, const char *dataPtr, bool updateLCD)
{
    appglcdData.pointerX1 = x;
    appglcdData.pointerY1 = y;
    appglcdData.dataPtr = (uint8_t*)dataPtr;
    appglcdData.updateLCD = updateLCD;
    appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_TINY;
}

/** Reference functions for smaller letters, but not as small as Tiny ***/
//void LCDChrXY_Tiny(unsigned char x, unsigned char y, unsigned char ch)
//{
//    const unsigned char *glyph = FontLookup[ch - 32];
//    for (unsigned char col = 0; col < 5; col++)
//    {
//        unsigned char byte = glyph[col];
//        for (unsigned char bit = 0; bit < 7; bit++)
//        {
//            if (byte & (1 << bit))
//            {
//                SetPixel(x + col, y + bit);
//            }
//        }
//    }
//}
//void LCDStr_Tiny(unsigned char x, unsigned char y, const unsigned char *str)
//{
//    while (*str)
//    {
//        LCDChrXY_Tiny(x, y, *str);
//        x += 5; // solo 5px de ancho, sin espacio adicional
//        str++;
//    }
//    //LCDUpdate();
//}
/*****************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPGLCD_Initialize ( void )

  Remarks:
    See prototype in appglcd.h.
 */

void APPGLCD_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appglcdData.state = APPGLCD_STATE_INIT;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    appglcdData.adelay = RTC_Timer32CounterGet();
    LCD_RTS_Clear(); //Start resetting the GLCD module
    
    appglcdData.stateToReturn = APPGLCD_STATE_START_CONTRAST; //After the first update, the contrast must be calibrated.
}


/******************************************************************************
  Function:
    void APPGLCD_Tasks ( void )

  Remarks:
    See prototype in appglcd.h.
 */
void APPGLCD_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appglcdData.state )
    {
        /* Application's initial state. */
        case APPGLCD_STATE_INIT:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), appglcdData.adelay) > _1000ms)
            {
               appglcdData.adelay = RTC_Timer32CounterGet(); 
               LCD_RTS_Set(); 
               appglcdData.state = APPGLCD_STATE_WAIT_1000ms;    
            }    
            break;
        }
        case APPGLCD_STATE_WAIT_1000ms:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), appglcdData.adelay) > _1000ms)
            {
                appglcdData.pointerX1 = 0x00;
                appglcdData.state = APPGLCD_STATE_LCD_CONFIGURATION_COMMANDS;
            }
            break;
        }
        /* TODO: implement your application state machine.*/
        case APPGLCD_STATE_LCD_CONFIGURATION_COMMANDS:
        {
            if (IsSPI1TaskIdle ()) //I wait until the SERCOM1 task is idle
            {
                LCDSend(commandsInitializeGLCD[appglcdData.pointerX1], SEND_CMD);
                appglcdData.pointerX1++;
                if (appglcdData.pointerX1 >= sizeof(commandsInitializeGLCD))
                {    
                    appglcdData.state = APPGLCD_STATE_START_CLEANING_GLCD;
                }    
            }
            break;
        }
         /*** LCD clear and update process ***/
        case APPGLCD_STATE_START_CLEANING_GLCD:
        {
            if (IsSPI1TaskIdle ()) //I wait until the SERCOM1 task is idle
            {
                appglcdData.pointerX1 = 0x00;
                appglcdData.state = APPGLCD_STATE_CLEAR_LCD;
            }
            break;
        }
        case APPGLCD_STATE_CLEAR_LCD:
        {
            if (appglcdData.pointerX1 < sizeof(appglcdData.LcdMemory))
            {
                appglcdData.LcdMemory[appglcdData.pointerX1] = 0x00;
                appglcdData.pointerX1++;
            }
            else
            {
                appglcdData.pointerY1 = 0x00;
                appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
            }
            break;
        }
        /*** LCD update process ***/
        case APPGLCD_STATE_START_GLCD_UPDATE:
        {
            if (IsSPI1TaskIdle ()) //I wait until the SERCOM1 task is idle
            {
                if (appglcdData.pointerY1 < (48/8))
                {
                    LCDSend(0x80, SEND_CMD);
                    appglcdData.state = APPGLCD_STATE_WRITE_Y_COORDINATES;  
                }
                else
                {
                    appglcdData.state = appglcdData.stateToReturn; //Here it returns to another state, so when the update process is called, this variable must have the next state
                }
            }
            break;
        }
        case APPGLCD_STATE_WRITE_Y_COORDINATES:
        {
            if (IsSPI1TaskIdle ()) //I wait until the SERCOM1 task is idle
            {
                LCDSend(0x40 | (uint8_t)(appglcdData.pointerY1), SEND_CMD);
                appglcdData.state = APPGLCD_STATE_WRITE_X_COORDINATES; 
                appglcdData.pointerX1 = 0x00;
            }
            break;
        }
        case APPGLCD_STATE_WRITE_X_COORDINATES:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                if (appglcdData.pointerX1 < 84)
                {
                    LCDSend(appglcdData.LcdMemory[appglcdData.pointerY1 * 84 + appglcdData.pointerX1], SEND_CHR);
                    appglcdData.pointerX1++;
                }
                else
                {
                    appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
                    appglcdData.pointerY1++;
                }
            }
            break;
        }
        /*** End of update process code ****/
        /*** LCD contrast calibration start ***/
        case APPGLCD_STATE_START_CONTRAST:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                LCDSend(0x21, SEND_CMD);
                appglcdData.state = APPGLCD_STATE_SET_LCD_VOP;
            }
            break;
        }
        case APPGLCD_STATE_SET_LCD_VOP:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                LCDSend(0x80 | LCD_CONTRAST, SEND_CMD);
                appglcdData.state = APPGLCD_STATE_HORIZONTAL_ADDRESSING_MODE;
            }
            break;
        }
        case APPGLCD_STATE_HORIZONTAL_ADDRESSING_MODE:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                LCDSend(0x20, SEND_CMD);
                appglcdData.state = APPGLCD_STATE_START_CLEANING_GLCD; //Once the contrast is set, the LCD must be updtae
                appglcdData.stateToReturn = APPGLCD_STATE_IDLE; //And after the update, it should go into idel state until another task asks it to plot something.
            }
            break;
        }
        /*** End of LCD contrast calibration code ***/
        case APPGLCD_STATE_IDLE: break;
        case APPGLCD_STATE_GRAPH_LINE:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                LCDPixelXY (appglcdData.pointerX1, appglcdData.pointerY1);
                if ((appglcdData.pointerX1 == appglcdData.pointerX2) && (appglcdData.pointerY1 == appglcdData.pointerY2))
                {
                    appglcdData.state = APPGLCD_STATE_WAIT_PROCESS_COMPLETION_GLCD; 
                }
                else
                {
                    appglcdData.error2 = 2*appglcdData.error1;
                    if (appglcdData.error2 > -appglcdData.dy) 
                    {
                        appglcdData.error1 = appglcdData.error1 - appglcdData.dy;
                        appglcdData.pointerX1 = appglcdData.pointerX1 + appglcdData.sx;
                    }
                    if (appglcdData.error2 < appglcdData.dx) 
                    {
                        appglcdData.error1 = appglcdData.error1 + appglcdData.dx;
                        appglcdData.pointerY1 = appglcdData.pointerY1 + appglcdData.sy;
                    }
                }
            }
            break;
        }
        case APPGLCD_STATE_WAIT_PROCESS_COMPLETION_GLCD:
        {
            if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
            {
                appglcdData.pointerY1 = 0x00;
                appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
                appglcdData.stateToReturn = APPGLCD_STATE_IDLE; //Once you update the LCD, it should return to idle state.
            }
            break;
        }
        case APPGLCD_STATE_START_DRAW_LOGO:
        {
           if (IsSPI1TaskIdle()) //I wait until the SERCOM1 task is idle
           {
                if (appglcdData.pointerY1 < 0x08)
                {
                    if (appglcdData.logoSample & 0x80) 
                    {
                        LCDPixelXY(appglcdData.col + appglcdData.pointerY1, appglcdData.row);
                    }
                    appglcdData.logoSample <<= 1;
                    appglcdData.pointerY1++;
                }
                else
                {
                    appglcdData.state = APPGLCD_STATE_DRAW_LOGO;
                }
           } 
           break;
        }
        case APPGLCD_STATE_DRAW_LOGO:
        {
            if (IsSPI1TaskIdle())
            {
                appglcdData.col += 8;
                if (appglcdData.col >= 84) 
                {
                    appglcdData.col = 0;
                    appglcdData.row += 1;
                }
                if (appglcdData.pointerX1 < sizeof(logoSicoy))
                {
                    appglcdData.pointerX1++;
                    appglcdData.logoSample = logoSicoy[appglcdData.pointerX1];
                    appglcdData.pointerY1 = 0;
                    appglcdData.state = APPGLCD_STATE_START_DRAW_LOGO;
                }
                else
                {
                    appglcdData.pointerY1 = 0x00;
                    appglcdData.state = APPGLCD_STATE_START_GLCD_UPDATE;
                    appglcdData.stateToReturn = APPGLCD_STATE_IDLE; 
                }
            }
            break;
        }
        case APPGLCD_STATE_START_WRITE_MESSAGE_ROW:
        {
            if (IsSPI1TaskIdle())
            {
                if (appglcdData.xMessage > LCD_X_RES || appglcdData.row > LCD_Y_RES) // check for out off range
                {
                    appglcdData.stateToReturn = APPGLCD_STATE_IDLE;
                }
                else  if (*appglcdData.dataPtr) 
                {
                    appglcdData.pointerX1 = (uint32_t) appglcdData.xMessage * 6 + (uint32_t) appglcdData.row * 84;
                    appglcdData.pointerY1 = 0x00;
                    appglcdData.state = APPGLCD_STATE_WRITE_MESSAGE_ROW;
                }
                else
                {
                    appglcdData.pointerY1 = 0x00;
                    updateLCDOrIdleState();
                }
            }
            break;
        }
        case APPGLCD_STATE_WRITE_MESSAGE_ROW:
        {
            if (appglcdData.pointerY1 < 0x06)
            {
                if (0x05 == appglcdData.pointerY1)
                {
                    if (appglcdData.inv)
                    {
                        appglcdData.LcdMemory[appglcdData.pointerX1++] = 0xFF;
                    }
                    else
                    {
                        appglcdData.LcdMemory[appglcdData.pointerX1++] = 0x00;
                    }
                }
                else
                {
                    uint8_t f;
                    if (appglcdData.inv)
                    {
                        f = ~(FontLookup[*appglcdData.dataPtr - 32][appglcdData.pointerY1]);
                    }
                    else
                    {
                        f = FontLookup[*appglcdData.dataPtr - 32][appglcdData.pointerY1];
                    }
                    appglcdData.LcdMemory[appglcdData.pointerX1++] = f;
                }
                appglcdData.pointerY1++;
            }
            else
            {
                appglcdData.xMessage++;
                appglcdData.dataPtr++; 
                appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_ROW;
            }
            break;
        }
        case APPGLCD_STATE_START_WRITE_MESSAGE_SCALED:
        {
            if (IsSPI1TaskIdle())
            {
                if (*appglcdData.dataPtr) 
                {
                    appglcdData.glyph = FontLookup[*appglcdData.dataPtr - 32];
                    appglcdData.pointerX2 = 0x00;
                    appglcdData.pointerY2 = 0x00;
                    appglcdData.bit = 0x00;
                    appglcdData.col = 0x00;
                    appglcdData.state = APPGLCD_STATE_WRITE_MESSAGE_SCALED;
                }
                else
                {
                    updateLCDOrIdleState();
                }
            }
            break;
        }
        case APPGLCD_STATE_WRITE_MESSAGE_SCALED:
        {
            if (IsSPI1TaskIdle())
            {
                if (appglcdData.col < 0x05)
                {
                    uint8_t byte = appglcdData.glyph[appglcdData.col];
                    if (appglcdData.bit < 0x07)
                    {
                        if (byte & (1 << appglcdData.bit))
                        {
                            if (appglcdData.pointerY2 < appglcdData.scale)
                            {
                                if (appglcdData.pointerX2 < appglcdData.scale)
                                {
                                    SetPixel(appglcdData.pointerX1 + appglcdData.col * appglcdData.scale + appglcdData.pointerX2, appglcdData.pointerY1 + appglcdData.bit * appglcdData.scale + appglcdData.pointerY2);
                                    appglcdData.pointerX2++;
                                }
                                else
                                {
                                    appglcdData.pointerX2 = 0x00;
                                    appglcdData.pointerY2++;
                                }
                                return;
                            }
                        }
                        appglcdData.pointerX2 = 0x00;
                        appglcdData.pointerY2 = 0x00;
                        appglcdData.bit++;
                        return;
                    }
                    appglcdData.pointerX2 = 0x00;
                    appglcdData.pointerY2 = 0x00;
                    appglcdData.bit = 0x00;
                    appglcdData.col++;
                }
                else
                {
                    appglcdData.pointerX1 += 6 * appglcdData.scale; // 5 columnas + 1 espacio
                    appglcdData.dataPtr++; 
                    appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_SCALED;
                }
            }
            break;
        }
        case APPGLCD_STATE_START_WRITE_MESSAGE_TINY:
        {
            if (IsSPI1TaskIdle())
            {
                if (*appglcdData.dataPtr) 
                {
                    if (*appglcdData.dataPtr < 32 || *appglcdData.dataPtr > 126)
                    {
                        appglcdData.state = APPGLCD_STATE_IDLE;// outside the printable ASCII range
                    }
                    else
                    {
                        appglcdData.glyph = TinyFont[*appglcdData.dataPtr - 32];
                        appglcdData.col = 0x00;
                        appglcdData.bit = 0x00;
                        appglcdData.state = APPGLCD_STATE_WRITE_MESSAGE_TINY;
                    }
                }
                else
                {
                    updateLCDOrIdleState();
                }
            }
            break;
        }
        case APPGLCD_STATE_WRITE_MESSAGE_TINY:
        {
            if (IsSPI1TaskIdle())
            {
                if (appglcdData.col < 0x03)
                {
                    uint8_t bits = appglcdData.glyph[appglcdData.col];
                    if (appglcdData.bit < 0x05)
                    {
                        if (bits & (1 << appglcdData.bit))
                        {
                            SetPixel(appglcdData.pointerX1 + appglcdData.col, appglcdData.pointerY1 + appglcdData.bit);
                        }
                        else
                        {
                            ClearPixel(appglcdData.pointerX1 + appglcdData.col, appglcdData.pointerY1 + appglcdData.bit);
                        }
                        appglcdData.bit++;
                    }
                    else
                    {
                        appglcdData.bit = 0x00;
                        appglcdData.col++;
                    }
                    return; 
                }
                else
                {
                    appglcdData.pointerX1 += 4; // 3 px + 1 espacio
                    appglcdData.dataPtr++; 
                    appglcdData.state = APPGLCD_STATE_START_WRITE_MESSAGE_TINY;
                }
            }
            break;
        }
        case APPGLCD_STATE_START_CLEAR_MESSAGE_SCALED:
        {
            if (IsSPI1TaskIdle())
            {
                if (appglcdData.bit < appglcdData.pointerX2)
                {
                    appglcdData.sx = appglcdData.pointerX1 + appglcdData.bit*6*appglcdData.scale;
                    if (appglcdData.dy < (7*appglcdData.scale))
                    {
                        if (appglcdData.dx < (5*appglcdData.scale))
                        {
                            ClearPixel((uint8_t)(appglcdData.sx + appglcdData.dx), (uint8_t)(appglcdData.pointerY1 + appglcdData.dy));
                            appglcdData.dx++;
                        }
                        else
                        {
                            appglcdData.dx = 0x00;
                            appglcdData.dy++;
                        }
                        return;
                    }
                    else
                    {
                        appglcdData.dy = 0x00;
                        appglcdData.bit++;
                    }
                    return;
                }
                else
                {
                    updateLCDOrIdleState();
                }
            }
            break;
        }
        /* The default state should never be executed. */
        default: break; /* TODO: Handle error in application's state machine. */
    }
}


/*******************************************************************************
 End of File
 */
