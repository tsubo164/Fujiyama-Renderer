// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <fstream>
#include <string>
#include <vector>

namespace fj {

#define DEFINE_READ_WRITE(type) \
inline void read_(std::ifstream &file, type &dst) \
{ \
  file.read(reinterpret_cast<char*>(&dst), sizeof(dst)); \
} \
inline void write_(std::ofstream &file, const type &src) \
{ \
  file.write(reinterpret_cast<const char*>(&src), sizeof(src)); \
}

DEFINE_READ_WRITE(char)
DEFINE_READ_WRITE(unsigned char)
DEFINE_READ_WRITE(short)
DEFINE_READ_WRITE(unsigned short)
DEFINE_READ_WRITE(int)
DEFINE_READ_WRITE(unsigned int)
DEFINE_READ_WRITE(long)
DEFINE_READ_WRITE(unsigned long)
DEFINE_READ_WRITE(float)
DEFINE_READ_WRITE(double)

#undef DEFINE_READ_WRITE

// Vector 
inline void read_(std::ifstream &file, Vector &dst)
{
  read_(file, dst.x);
  read_(file, dst.y);
  read_(file, dst.z);
}

inline void write_(std::ofstream &file, const Vector &src)
{
  write_(file, src.x);
  write_(file, src.y);
  write_(file, src.z);
}

// Color 
inline void read_(std::ifstream &file, Color &dst)
{
  read_(file, dst.r);
  read_(file, dst.g);
  read_(file, dst.b);
}

inline void write_(std::ofstream &file, const Color &src)
{
  write_(file, src.r);
  write_(file, src.g);
  write_(file, src.b);
}

// TexCoord 
inline void read_(std::ifstream &file, TexCoord &dst)
{
  read_(file, dst.u);
  read_(file, dst.v);
}

inline void write_(std::ofstream &file, const TexCoord &src)
{
  write_(file, src.u);
  write_(file, src.v);
}

// std::string 
inline void read_(std::ifstream &file, std::string &dst)
{
  dst = "";
  for (;;) {
    char c = '\0';
    read_(file, c);
    if (c == '\0') {
      break;
    }
    dst += c;
  }
}

inline void write_(std::ofstream &file, const std::string &src)
{
  for (std::string::const_iterator it = src.begin(); it != src.end(); ++it) {
    write_(file, *it);
  }
  write_(file, '\0');
}

// std::vector<T> 
template<typename T>
inline void read_(std::ifstream &file, std::vector<T> &dst)
{
  std::size_t count = 0;
  read_(file, count);
  dst.resize(count);
  for (typename std::vector<T>::iterator it = dst.begin(); it != dst.end(); ++it) {
    read_(file, *it);
  }
}

template<typename T>
inline void write_(std::ofstream &file, const std::vector<T> &src)
{
  const std::size_t count = src.size();
  write_(file, count);
  for (typename std::vector<T>::const_iterator it = src.begin(); it != src.end(); ++it) {
    write_(file, *it);
  }
}

} // namespace xxx
