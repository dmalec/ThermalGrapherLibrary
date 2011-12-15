// --------------------------------------------------------------------------------
// Copyright (C) 2011 Dan Malec
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// --------------------------------------------------------------------------------
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
 #include "WConstants.h"
#endif
#include "ThermalGrapher.h"

#define BUFFER_HEIGHT 16
#define BUFFER_WIDTH_PXL 384
#define BUFFER_WIDTH_CHAR (BUFFER_WIDTH_PXL / 8)


//! Class constructor.
/*!
  The class constructor saves the configuration parameters and
  initializes a buffer to draw into.
  
  \param printer the Thermal instance to use.
  \param min the minimum value to expect for data points.
  \param max the maximum value to expect for data points.
 */
ThermalGrapher::ThermalGrapher(Thermal *printer, int min, int max) {
  _printer = printer;
  _min = min;
  _max = max;
  _oldValue = 0;
  _primed = false;

  // Allocate the buffer space.
  int row;
  _buffer = (uint8_t **)malloc(BUFFER_HEIGHT * sizeof(uint8_t *));
  for (row=0; row<BUFFER_HEIGHT; row++) {
    _buffer[row] = (uint8_t *)malloc(BUFFER_WIDTH_CHAR * sizeof(uint8_t));
  }
}

//! Class destructor.
/*!
  Deallocate the buffer.
*/
ThermalGrapher::~ThermalGrapher() {
  int row;

  for (row=0; row<BUFFER_HEIGHT; row++) {
    free(_buffer[row]);
  }
  free(_buffer);
}

//! Draw a value on the graph.
/*!
  Draws a line from the last point to this point, unless this is the first
  invocation.  In that case, simply stores the value for the next drawing
  cycle.

  \param value the value to graph
*/
void ThermalGrapher::printValue(int value) {
  int row, col;

  value = (int)(((float)(value - _min) / (float)(_max - _min)) * (float)(BUFFER_WIDTH_PXL - 4) + 2);

  if (!_primed) {
    _primed = true;
    _oldValue = value;
  } else {
    int row;
  
    for (row=0; row<BUFFER_HEIGHT; row++) {
      memset(_buffer[row], 0, BUFFER_WIDTH_CHAR);
      setPixel(0, row);
      setPixel(1, row);
        
      setPixel(BUFFER_WIDTH_PXL - 2, row);
      setPixel(BUFFER_WIDTH_PXL - 1, row);
    }
  
    drawLine(_oldValue, 0, value, BUFFER_HEIGHT - 1);

    _printer->write(18);
    _printer->write(42);
    _printer->write(BUFFER_HEIGHT);
    _printer->write(BUFFER_WIDTH_CHAR);

    for (row=0; row<BUFFER_HEIGHT; row++) {
      for (col=0; col<BUFFER_WIDTH_CHAR; col++) {
	_printer->write(_buffer[row][col]);
      }
    }

    _oldValue = value;
  }
}

//! Set an individual pixel in the buffer.
/*!
  \param x the x coordinate (left to right)
  \param y the y coordinate (top to bottom)
*/
void ThermalGrapher::setPixel(int x, int y) {
  _buffer[y][x / 8] |= (0x80 >> (x % 8));
}

//! Draw a line in the buffer.
/*!
  \param x0 the x0 coordinate (left to right)
  \param y0 the y0 coordinate (top to bottom)
  \param x1 the x1 coordinate (left to right)
  \param y1 the y1 coordinate (top to bottom)
*/
void ThermalGrapher::drawLine(int x0, int y0, int x1, int y1) {
  int dx = abs(x1-x0);
  int dy = abs(y1-y0);
  int sx = -1;
  int sy = -1;
  int err = dx - dy;
  int e2;

  if (x0 < x1) {
    sx = 1;
  }
  if (y0 < y1) {
    sy = 1;
  }

  while (x0 != x1 && y0 != y1) {
    setPixel(x0, y0);

    e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}
