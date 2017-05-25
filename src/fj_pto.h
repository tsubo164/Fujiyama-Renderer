// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PTO_H
#define FJ_PTO_H

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cstdio>

namespace fj {

const std::string PTO_HEADER = "#PTO Plain Text Object";

inline void WritePtoHeader(std::ostream &strm)
{
  strm << PTO_HEADER << std::endl;
}

inline bool ReadPtoHeader(std::istream &strm)
{
  std::string header;
  getline(strm, header);
  return header == PTO_HEADER;
}

class PtoParser {
public:
  PtoParser() {}
  virtual ~PtoParser() {}

  bool ParseHeader(std::istream &strm)
  {
    return ReadPtoHeader(strm);
  }

  void ParseBody(std::istream &strm)
  {
    parse_lines(strm);
  }

  int Parse(std::istream &strm)
  {
    if (!ParseHeader(strm)) {
      return -1;
    }

    ParseBody(strm);
    return 0;
  }

protected:
  long to_integer(const std::string &s, long otherwise)
  {
    char *endp = NULL;
    const long l = strtol(s.c_str(), &endp, 0);
    return *endp == '\0' ? l : otherwise;
  }

  double to_float(const std::string &s, float otherwise)
  {
    char *endp = NULL;
    const double f = strtod(s.c_str(), &endp);
    return *endp == '\0' ? f : otherwise;
  }

private:
  virtual void begin(const std::vector<std::string> &tokens) = 0;
  virtual void end(const std::vector<std::string> &tokens) = 0;
  virtual void process(const std::vector<std::string> &tokens) = 0;

  enum TokenKind {
    TK_value = 256,
    TK_string
  };

  std::vector<std::string> tokens;

  bool is_spaces(int c) {
    if (c == ' ' || c == '\t' || c == '\v') {
      return true;
    } else {
      return false;
    }
  }

  void skip_spaces(std::istream &strm)
  {
    while (is_spaces(strm.get())) {
      // does nothing
    }
    strm.unget();
  }

  int scan_string(std::istream &strm, std::string &token)
  {
    assert(strm.get() == '"');

    for (;;) {
      const int c = strm.get();
      switch (c) {
      case '"':
        if (!isspace(strm.peek())) {
          //error need white space(s) after '"'
          return -1;
        }
        return TK_string;
      case '\n':
        // error no '"' appears before end of line
        return -1;
      default:
        token += c;
        break;
      }
    }
  }

  int scan_value(std::istream &strm, std::string &token)
  {
    for (;;) {
      const int c = strm.get();
      if (is_spaces(c) || c == '\n' || c == EOF) {
        strm.unget();
        return TK_value;
      } else {
        token += c;
      }
    }
  }

  int scan_token(std::istream &strm, std::string &token)
  {
    skip_spaces(strm);

    switch (strm.peek()) {
    case '\n': case EOF:
      return strm.get();
    case '"':
      return scan_string(strm, token);
    default:
      return scan_value(strm, token);
    }
  }

  int tokenize_line(std::istream &strm, std::vector<std::string> &tokens)
  {
    static const int MAX_TOKENS = 256;
    std::string token;

    for (int i = 0; i < MAX_TOKENS; i++) {
      token.clear();
      const int tk = scan_token(strm, token);
      if (tk == '\n' || tk == EOF) {
        return tk;
      }
      tokens.push_back(token);
    }
    //error too many tokens in a line
    std::cout << "ERROR\n";
    return -1;
  }

  int parse_lines(std::istream &strm)
  {
    std::vector<std::string> tokens;

    for (;;) {
      tokens.clear();
      const int tk = tokenize_line(strm, tokens);
      if (tk == EOF) {
        break;
      }

      if (tokens.empty() || tokens[0][0] == '#') {
        continue;
      } else if (tokens[0] == "begin") {
        begin(tokens);
      } else if (tokens[0] == "end") {
        end(tokens);
      } else {
        process(tokens);
      }
    }

    return 0;
  }
};

} // namespace xxx

#endif // FJ_XXX_H
