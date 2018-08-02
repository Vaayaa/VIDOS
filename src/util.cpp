#ifndef UTIL_CPP
#define UTIL_CPP

#include <cassert>
//UTILITY CLASS
//---------------
// Helpful functions for OpenGL Shenanigans,
// should be generally applicable functions that are relevant throughout the codebase
// (e.g. Color space conversions, opengl error checking funcitons etc.)



//UTILITY: Create an unsigned short 16 bit 565 short color from 3 floats
//Turns a float color into an OpenGL GL_UNSIGNED_SHORT_5_6_5 color

static unsigned short ushortColor( float red, float green, float blue) {

  int r = red * 30;
  int g = green * 63;
  int b = blue * 30;


  assert (r < 31);
  assert (g < 64);
  assert (b < 31);
  assert (r > -1);
  assert (g > -1);
  assert (b > -1);

  unsigned short shortColor = 0;

  shortColor = shortColor + r;
  //std::cout<< "w/R:"<< shortColor<< std::endl;
  shortColor = shortColor << 6; //move 6 bits ( g is 6 bits )
  shortColor = shortColor + g;
  //std::cout<< "w/G:" << shortColor<< std::endl;
  shortColor = shortColor << 5; //move 5 bits ( r is 5 bits)
  shortColor = shortColor + b; // b is remaining 6 bits
  //std::cout<< "w/B:" << shortColor<< std::endl;

  return shortColor;

}


#endif