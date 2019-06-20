// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_GEO_IO_H
#define FJ_GEO_IO_H

#include "fj_types.h"
// TODO move to .cc
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_tex_coord.h"
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include <iostream>

namespace fj {

const char SIGNATURE[] = "fjgeo";
const size_t SIGNATURE_SIZE = 8;

class Archive {
public:
  typedef int64_t SizeType;
  typedef int64_t IntType;
  typedef double  FloatType;

public:
  Archive();
  ~Archive();

  int OpenOutput(const std::string &filename);
  int OpenInput(const std::string &filename);

  void Close();

  void SetPositionData(
      const Vector *data, int64_t data_count,
      const int64_t *index, int64_t index_count)
  {
    position_.data        = const_cast<Vector *>(data);
    position_.data_count  = data_count;
    position_.index       = const_cast<int64_t *>(index);
    position_.index_count = index_count;
  }

  void SetNormalData(
      const Vector *data, int64_t data_count,
      const int64_t *index, int64_t index_count)
  {
    normal_.data        = const_cast<Vector *>(data);
    normal_.data_count  = data_count;
    normal_.index       = const_cast<int64_t *>(index);
    normal_.index_count = index_count;
  }

  int64_t GetPositionCount() const
  {
    return position_.data_count;
  }

  void Write();

  bool IsFailed() const;

  //======================================================================
  std::string ReadDataName();
  void ReadData();

  void SkipData()
  {
    size_t size = 0;
    file_.read(reinterpret_cast<char*>(&size), sizeof(size));
    file_.seekg(size, std::fstream::cur);
  }

private:
  Archive(const Archive &);
  const Archive &operator=(const Archive &);

  std::string read_data_name();
  void read_data_info(char *data_type, char *data_size,
      char *elem_size, int64_t *data_count);
  void read_data(int64_t *data, int64_t count);

  std::fstream file_;
  std::vector<char> string_buf_;

  template <typename T>
  struct DataRef {
    DataRef() :
        data(0),
        data_count(0),
        index(0),
        index_count(0) {}
    ~DataRef() {}

    T *data;
    int64_t data_count;
    int64_t *index;
    int64_t index_count;
  };

  DataRef<Vector>   position_;
  DataRef<Vector>   normal_;
  DataRef<TexCoord> texture_;
  DataRef<Color>    color_;

  int err_;

  //======================================================================
  char data_type_;
  char data_size_;
  char elem_size_;
};

} // namespace xxx

#endif // FJ_XXX_H
