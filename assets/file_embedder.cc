#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

string toVariableName(string path) {
  string filename = filesystem::path(path).filename().string();
  for (auto &c : filename) {
    c = (isalpha(c) || isdigit(c) || c == '_') ? toupper(c) : '_';
  }
  return filename;
}

const char lookupTable[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

string toHex(int i) {
  unsigned char c = (unsigned char)i;
  return string({'0', 'x', lookupTable[(c & 0xf0) >> 4], lookupTable[c & 0xf]});
}

typedef tuple<string, string, size_t> info;
info putFile(ofstream &stream, const char *filename) {
  cerr << "      - " << filename << endl;
  string name = toVariableName(filename);
  stream << "static const unsigned char " << name << "[] = {";
  ifstream input;
  input.open(filename);
  size_t count = 0;
  while (input.good()) {
    int c = input.get();
    if (count % 16 == 0) {
      stream << "\n  ";
    }
    stream << toHex(c) << ", ";
    ++count;
  }
  stream << "\n};\nstatic const size_t " << name << "_SIZE = " << count
         << ";\n\n";
  return info(filesystem::path(filename).filename(), name, count);
}

int main(int argc, char **argv) {
  if (argc > 2) {
    cerr << "    Embedding " << argc - 2 << " file(s) into " << quoted(argv[1])
         << endl;
    ofstream stream;
    stream.open(argv[1]);
    stream << "#include <cstdlib>\n"
           << "#include \"asset_manager.h\"\n"
           << "#ifndef IMPORT_BUILTIN_ASSETS\n"
           << "#error \"Do not include this file\"\n"
           << "#endif\n\n";
    vector<info> assets;
    for (int i = 2; i < argc; ++i) {
      const char *path = argv[i];
      if (filesystem::is_directory(path)) {
        for (auto &f : filesystem::directory_iterator(filesystem::path(path))) {
          if (f.is_regular_file()) {
            assets.push_back(putFile(stream, f.path().c_str()));
          }
        }
      } else if (filesystem::is_regular_file(path)) {
        assets.push_back(putFile(stream, path));
      } else {
        cerr << "      * Ignoring " << path << endl;
      }
    }
    stream << "static const AssetInfo BUNDLED_ASSETS[] = {\n";
    for (auto &i : assets) {
      stream << "  { " << quoted(get<0>(i)) << ", " << get<1>(i) << ", "
             << get<2>(i) << " },\n";
    }
    stream << "};\n";
    stream.flush();
    stream.close();
    return 0;
  } else {
    return -1;
  }
}