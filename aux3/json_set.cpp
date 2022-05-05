#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>

using json = nlohmann::json;

int main(int argc, const char** argv) {

  if (argc < 3) {
    std::cout << "Usage: read_json <file> <name> <value>\n";
    return -1;
  }

  std::fstream fs {argv[1]};

  json obj;
  fs >> obj;

  obj[argv[2]] = json::parse(argv[3]);

  fs.seekp(0);
  fs << obj.dump(2) << std::endl;

  return 0;
}
