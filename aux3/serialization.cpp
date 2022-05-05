#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

struct Vector3 {
  float x;
  float y;
  float z;
};

struct VisualData {
  int vao;
  int shader_program;
};

struct TransformData {
  Vector3 position;
  Vector3 rotation;
  Vector3 scale;
};

struct GameObject {
  TransformData transform;
  VisualData visual;
};

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)

void to_json(json& j, const Vector3& v) {
  j = {v.x, v.y, v.z};
}

void from_json(const json& j, Vector3& v) {
  v.x = j.at(0);
  v.y = j.at(1);
  v.z = j.at(2);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VisualData, vao, shader_program)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TransformData, position, rotation, scale)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameObject, transform, visual)

int main() {

  GameObject game_object {
    { // transform
      {0.0, 1.0, 2.0},
      {45.0, .0, .0},
      {1.0, 1.0, 1.0}
    },
    { // visual
      3,
      42
    }
  };

  std::ofstream output_file {"data.json"};

  json serialized_game_object {game_object};

  output_file << serialized_game_object.dump(2) << std::endl;

  return 0;
}
