#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

struct Vector3 {
  float x {0.0f};
  float y {0.0f};
  float z {0.0f};
};

struct Transform {
  Vector3 position  {0.0f, 0.0f, 0.0f};
  Vector3 rotation  {0.0f, 0.0f, 0.0f};
  Vector3 scale     {1.0f, 1.0f, 1.0f};
};

// json j = Vector3();
void to_json(json& j, const Vector3& v) {
  j = {v.x, v.y, v.z};
}

// Vector3 v = j;
void from_json(const json& j, Vector3& v) {
  v.x = j.at(0);
  v.y = j.at(1);
  v.z = j.at(2);
}

//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, position, rotation, scale)

/*
{
"position" : ...,
"rotation" : ...,
"scale"    : ...
}
*/

std::ostream& operator<< (std::ostream& os, const Vector3& v) {
  return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

int main(int argc, char** argv) {

  if (argc < 2) {
    std::cout << "usage: serialization <file>" << std::endl;
    return -1;
  }

  Transform tr;

  try {
    std::ifstream file {argv[1]};
    json j;
    file >> j;
    tr = j;
  } catch (std::exception &e) {
    std::cerr << "A parsing error occurred: " << e.what() << std::endl;
  }

  std::cout << "position  = " << tr.position << std::endl
            << "rotation  = " << tr.rotation << std::endl
            << "scale     = " << tr.scale << std::endl;

  std::cout << "¿modificar posición? [y/n]" << std::endl;

  char answer;
  std::cin >> answer;

  auto yes = [](char c) { return c == 'y' or c == 'Y'; };

  if (yes(answer)) {
    std::cout << "x=";
    std::cin >> tr.position.x;

    std::cout << "y=";
    std::cin >> tr.position.y;

    std::cout << "z=";
    std::cin >> tr.position.z;
  }

  {
    std::ofstream file {argv[1], std::ios::out | std::ios::trunc};
    json j = tr;
    file.seekp(0);
    file << j.dump(2) << std::endl;
  }

  return 0;
}
