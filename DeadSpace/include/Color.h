/*
 * Color.h
 *
 *  Created on: Jan 5, 2016
 *      Author: sidlausk
 */

#ifndef COLOR_H_
#define COLOR_H_

enum ColorType {
  WHITE_COLOR,
  BLACK_COLOR,
  RED_COLOR,
  GREEN_COLOR,
  BLUE_COLOR,
  YELLOW_COLOR,
  GREY_COLOR
};

class Color {
public:
  unsigned char rgb[3];
  Color(ColorType c) {
    switch (c) {
    case BLACK_COLOR:
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
      break;
    case WHITE_COLOR:
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 255;
      break;
    case RED_COLOR:
      rgb[0] = 255;
      rgb[1] = 0;
      rgb[2] = 0;
      break;
    case GREEN_COLOR:
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 0;
      break;
    case BLUE_COLOR:
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 255;
      break;
    case YELLOW_COLOR:
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 0;
      break;
    case GREY_COLOR:
      rgb[0] = 125;
      rgb[1] = 125;
      rgb[2] = 125;
      break;
    default:
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
    }
  }
  Color(unsigned char R, unsigned char G, unsigned char B) {
    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;
  }
};

#endif /* COLOR_H_ */
