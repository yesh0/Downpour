#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

string toVariableName(string path) {
  string filename = filesystem::path(path).filename().string();
  for (auto &c : filename) {
    c = isalpha(c) ? toupper(c) : '_';
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

void putFile(ofstream &stream, const char *filename) {
  cerr << "      - " << filename << endl;
  string name = toVariableName(filename);
  stream << "const unsigned char " << name << "[] = {";
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
  stream << "\n};\nconst size_t " << name << "_SIZE = " << count << ";\n\n";
}

int main(int argc, char **argv) {
  if (argc > 2) {
    cerr << "    Embedding " << argc - 2 << " file(s) into " << quoted(argv[1])
         << endl;
    ofstream stream;
    stream.open(argv[1]);
    stream << "#include <cstdlib>\n"
           << "#ifndef IMPORT_BUILTIN_ASSETS\n"
           << "#error \"Do not include this file\"\n"
           << "#endif\n\n";
    for (int i = 2; i < argc; ++i) {
      const char * path = argv[i];
      if (filesystem::is_directory(path)) {
        for (auto & f : filesystem::directory_iterator(filesystem::path(path))) {
          if (f.is_regular_file()) {
            putFile(stream, f.path().c_str());
          }
        }
      } else if (filesystem::is_regular_file(path)) {
        putFile(stream, path);
      } else {
        cerr << "      * Ignoring " << path << endl;
      }
    }
    stream.flush();
    stream.close();
    return 0;
  } else {
    return -1;
  }
}