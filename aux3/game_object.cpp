#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>

using json = nlohmann::json;

struct Vector3 {
  float x, y, z;
};

void to_json(json& j, const Vector3& v) {
  j = {v.x, v.y, v.z};
}

void from_json(const json& j, Vector3& v) {
  v.x = j.at(0);
  v.y = j.at(1);
  v.z = j.at(2);
}


struct Transform {
  Vector3 position  {0.0f, 0.0f, 0.0f};
  Vector3 rotation  {0.0f, 0.0f, 0.0f};
  Vector3 scale     {1.0f, 1.0f, 1.0f};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, position, rotation, scale)


struct Visual {
  int id;
  int mesh;
  int shader;
};

std::map<int, Visual*> visual_map;

void to_json(json& j, const Visual& v) {
  j = {{"id", v.id}, {"mesh", v.mesh}, "shader", v.shader};
}

void from_json(const json& j, Visual& v) {
  v.id = j.at("id");
  v.mesh = j.at("mesh");
  v.shader = j.at("shader");
}


struct GameObject {
  Transform tr;
  Visual *visual;
};

void to_json(json& j, const GameObject& o) {
  j["tr"] = o.tr;
  j["visual_id"] = o.visual->id;
}

void from_json(const json& j, GameObject& o) {
  o.tr = j.at("tr");
  o.visual = visual_map.at(j.at("visual_id").get<int>());
}

int main(int argc, char** argv) {
  json jdata;
  std::ifstream file {argv[1]};
  file >> jdata;

  Visual visual = jdata["visuals"][2];
  visual_map[visual.id] = &visual;  // <<<<

  GameObject go = jdata["game_objects"][0];

  std::cout << "Visual id: " << go.visual->id << std::endl;

  //json j = go;

  //std::cout << j.dump(2) << std::endl;

  return 0;
}
