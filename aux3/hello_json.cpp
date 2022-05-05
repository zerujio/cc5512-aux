#include <nlohmann/json.hpp>

#include <iostream>

using json = nlohmann::json;

int main() {
  json object;

  object["a_string"] = "Hello world!";
  object["a_number"] = 256;
  object["an_array"] = {1, 4, 6};

  // serialization
  std::cout << object.dump(2) << std::endl;
}
