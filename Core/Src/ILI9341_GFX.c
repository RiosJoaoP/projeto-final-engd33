#include <stdbool.h>
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#define __SWAP(a,b) { __typeof__(a) temp; temp = a; a = b; b = temp; }

void ILI9341_DrawHollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	float negCalc = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	//DRAW HORIZONTAL!
	if(!negX)
	{
		xLen = X1 - X0;
	}
	else
	{
		xLen = X0 - X1;
	}
	ILI9341_DrawHLine(X0, Y0, xLen, color);
	ILI9341_DrawHLine(X0, Y1, xLen, color);

	//DRAW VERTICAL!
	if(!negY)
	{
		yLen = Y1 - Y0;
	}
	else
	{
		yLen = Y0 - Y1;
	}

	ILI9341_DrawVLine(X0, Y0, yLen, color);
	ILI9341_DrawVLine(X1, Y0, yLen, color);

	if((xLen > 0)||(yLen > 0))
	{
		ILI9341_DrawPixel(X1, Y1, color);
	}
}

void ILI9341_DrawFilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	int32_t negCalc = 0;
	uint16_t X0True = 0;
	uint16_t Y0True = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	if(!negX)
	{
		xLen = X1 - X0;
		X0True = X0;
	}
	else
	{
		xLen = X0 - X1;
		X0True = X1;
	}

	if(!negY)
	{
		yLen = Y1 - Y0;
		Y0True = Y0;
	}
	else
	{
		yLen = Y0 - Y1;
		Y0True = Y1;
	}

	ILI9341_DrawRectangle(X0True, Y0True, xLen, yLen, color);
}

void ILI9341_DrawChar(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	if ((ch < 31) || (ch > 127)) return;

	uint8_t fOffset, fWidth, fHeight, fBPL;
	uint8_t *tempChar;

	fOffset = font[0];
	fWidth = font[1];
	fHeight = font[2];
	fBPL = font[3];

	tempChar = (uint8_t*)&font[((ch - 0x20) * fOffset) + 4]; /* Current Character = Meta + (Character Index * Offset) */

	/* Clear background first */
	ILI9341_DrawRectangle(X, Y, fWidth, fHeight, bgcolor);

	for (int j=0; j < fHeight; j++)
	{
		for (int i=0; i < fWidth; i++)
		{
			uint8_t z =  tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1]; /* (j & 0xF8) >> 3, increase one by 8-bits */
			uint8_t b = 1 << (j & 0x07);
			if (( z & b ) != 0x00)
			{
				ILI9341_DrawPixel(X+i, Y+j, color);
			}
		}
	}
}

void ILI9341_DrawText(const char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	uint8_t charWidth;			/* Width of character */
	uint8_t fOffset = font[0];	/* Offset of character */
	uint8_t fWidth = font[1];	/* Width of font */

	while (*str)
	{
		ILI9341_DrawChar(*str, font, X, Y, color, bgcolor);

		/* Check character width and calculate proper position */
		uint8_t *tempChar = (uint8_t*)&font[((*str - 0x20) * fOffset) + 4];
		charWidth = tempChar[0];

		if(charWidth + 2 < fWidth)
		{
			/* If character width is smaller than font width */
			X += (charWidth + 2);
		}
		else
		{
			X += fWidth;
		}

		str++;
	}
}

void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  int16_t dx, dy;

  if(x1 > x0)
	  dx = x1 - x0;
  else
	  dx = x0 - x1;

  if(y1 > y0)
	  dy = y1 - y0;
  else
	  dy = y0 - y1;

  int16_t x, y;
  int16_t err;
  int16_t step;

  if (0 == dx) {
    // vertical line
    if (0 == dy)
      { return; } // distance = 0, no line to draw
	ILI9341_DrawVLine(x0, y0, dy, color);
    return;
  }
  else if (0 == dy) {
    // horizontal line
    if (0 == dx)
      { return; } // distance = 0, no line to draw
    ILI9341_DrawHLine(x0, y0, dx, color);
    return;
  }

  bool is_steep = dy > dx;
  if (is_steep) {
    __SWAP(x0, y0);
    __SWAP(x1, y1);
  }

  if (x0 > x1) {
    __SWAP(x0, x1);
    __SWAP(y0, y1);
  }

  dx = x1 - x0;

  if(y1 > y0)
	  dy = y1 - y0;
  else
	  dy = y0 - y1;

  err = dx >> 1;

  if (y0 < y1)
    { step = 1; }
  else
    { step = -1; }

  while (x0 <= x1) {

    if (is_steep)
      { x = y0; y = x0; }
    else
      { x = x0; y = y0; }

    // continue algorithm even if current pixel is outside of screen
    // bounds, so that the line is drawn at correct position once
    // it actually enters screen bounds (if ever).
    if ( (x >= 0) && (x <= 320) && (y >= 0) && (y <= 240) ) {
      ILI9341_DrawPixel(x, y, color);
    }

    err -= dy;
    if (err < 0) {
      y0 += step;
      err += dx;
    }

    ++x0;
  }
}
