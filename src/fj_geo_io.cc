// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_geo_io.h"

namespace fj {

enum {
  Type_Null,
  Type_Integer,
  Type_Float,
  Type_String
};

template <typename T> struct DataType { static const int value = Type_Integer; };
template <> struct DataType<float>  { static const int value = Type_Float; };
template <> struct DataType<double> { static const int value = Type_Float; };
template <> struct DataType<std::string> { static const int value = Type_String; };
template <> struct DataType<Vector> { static const int value = Type_Float; };

template <typename T> struct DataSize { static const int value = sizeof(T); };
template <> struct DataSize<Vector>  { static const int value = sizeof(Real); };

template <typename T> struct ElementSize { static const int value = 1; };
template <> struct ElementSize<Vector> { static const int value = 3; };

template <typename T> inline
static void write_data(std::fstream &file,
    const std::string &name, const T *data, int64_t count)
{
  // data name
  const Archive::SizeType name_size = name.length() + 1;
  file.write(reinterpret_cast<const char*>(&name_size), sizeof(name_size));
  file.write(name.c_str(), sizeof(char) * name_size);

  if (data == NULL) {
    const char null_data_type = Type_Null;
    file.write(&null_data_type, sizeof(null_data_type));
    return;
  }

  // data info
  const char data_type = DataType<T>::value;
  const char data_size = DataSize<T>::value;
  const char elem_size = ElementSize<T>::value;
  file.write(&data_type, sizeof(data_type));
  file.write(&data_size, sizeof(data_size));
  file.write(&elem_size, sizeof(elem_size));
  file.write(reinterpret_cast<const char *>(&count), sizeof(count));

  // data body
  const Archive::SizeType size = data_size * elem_size * count;
  file.write(reinterpret_cast<const char*>(data), size);
}

static void write_null_data(std::fstream &file, const std::string &name)
{
  write_data(file, name, reinterpret_cast<const char *>(0), 0);
}

static bool match_signature(std::istream &istr, const char *signature)
{
  char sign[SIGNATURE_SIZE] = {'\0'};

  istr.read(sign, SIGNATURE_SIZE);
  return strcmp(sign, signature) == 0;
}

Archive::Archive() : err_(0)
{
}

Archive::~Archive()
{
  Close();
}

int Archive::OpenOutput(const std::string &filename)
{
  file_.open(filename.c_str(), std::fstream::out | std::fstream::binary);
  if (file_) {
    return -1;
  }
  return 0;
}

int Archive::OpenInput(const std::string &filename)
{
  file_.open(filename.c_str(), std::fstream::in | std::fstream::binary);
  if (!file_) {
    err_ = -1;
    return -1;
  }

  if (!match_signature(file_, "fjgeo")) {
    err_ = -1;
    return -1;
  }

  int64_t body_start;
  file_.read(reinterpret_cast<char *>(&body_start), sizeof(body_start));
  std::cout << "geo_io::body_start " << body_start << "\n";

  while (file_.tellg() < body_start) {

    std::string name = read_data_name();

    if (name == "position_count") {
      read_data(&position_.data_count, 1);
      std::cout << "geo_io::position_count " << position_.data_count << "\n";
    }
    else if (name == "normal_count") {
      read_data(&normal_.data_count, 1);
      std::cout << "geo_io::normal_count " << normal_.data_count << "\n";
    }
    else if (name == "end_of_header") {
      break;
    }
    else {
      break;
    }
  }

  return 0;
}

bool Archive::IsFailed() const
{
  // TODO BETTER WAY
  return err_ != 0;
}

void Archive::Close()
{
  file_.close();
}

void Archive::Write()
{
  char sign[SIGNATURE_SIZE] = {'\0'};
  strcpy(sign, SIGNATURE);
  file_.write(sign, SIGNATURE_SIZE);

  // dummy body start pos
  const int64_t header_start = file_.tellp();
  int64_t body_start = 0;
  file_.write(reinterpret_cast<const char *>(&body_start), sizeof(body_start));

  // header data
  write_data(file_, "position_count",       &position_.data_count,  1);
  write_data(file_, "position_index_count", &position_.index_count, 1);
  write_data(file_, "normal_count",         &normal_.data_count,    1);
  write_data(file_, "normal_index_count",   &position_.index_count, 1);
  write_data(file_, "texture_count",        &texture_.data_count,   1);
  write_data(file_, "texture_index_count",  &texture_.index_count,  1);
  write_data(file_, "color_count",          &color_.data_count,     1);
  write_data(file_, "color_index_count",    &color_.index_count,    1);

  // markerr
  write_null_data(file_, "end_of_header");

  // back patch body start pos
  body_start = file_.tellp();
  file_.seekp(header_start);
  file_.write(reinterpret_cast<const char *>(&body_start), sizeof(body_start));
  file_.seekp(body_start);

  // body data
  write_data(file_, "position_data",  &position_.data,  position_.data_count);
  //write_data(file_, "position_index", &position_.index, position_.index_count);
}

std::string Archive::read_data_name()
{
  SizeType size = 0;
  file_.read(reinterpret_cast<char*>(&size), sizeof(size));
  string_buf_.resize(size);
  file_.read(&string_buf_[0], sizeof(char) * size);
  return std::string(&string_buf_[0]);
}

void Archive::read_data_info(char *data_type, char *data_size,
    char *elem_size, int64_t *data_count)
{
  file_.read(data_type, sizeof(*data_type));
  file_.read(data_size, sizeof(*data_size));
  file_.read(elem_size, sizeof(*elem_size));
  file_.read(reinterpret_cast<char *>(data_count), sizeof(*data_count));
}

void Archive::read_data(int64_t *data, int64_t count)
{
  char data_type;
  char data_size;
  char elem_size;
  int64_t data_count;
  read_data_info(&data_type, &data_size, &elem_size, &data_count);

  const SizeType size = data_size * elem_size * data_count;
  string_buf_.resize(size);
  file_.read(&string_buf_[0], size);

  int64_t *tmp_data = reinterpret_cast<int64_t *>(&string_buf_[0]);
  for (int64_t i = 0; i < data_count; i++) {
    data[i] = tmp_data[i];
  }
}

std::string Archive::ReadDataName()
{
    return read_data_name();
}

void Archive::ReadData()
{
  int64_t data_count;
  read_data_info(&data_type_, &data_size_, &elem_size_, &data_count);

  const SizeType size = data_size_ * elem_size_ * data_count;
  string_buf_.resize(size);
  file_.read(&string_buf_[0], size);
}

} // namespace xxx
