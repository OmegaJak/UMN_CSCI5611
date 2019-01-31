#define _USE_MATH_DEFINES

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include "constants.h"
#include "goal.h"
#include "map.h"
#include "map_loader.h"
#include "spawn.h"
#include "wall.h"

using std::cout;
using std::endl;
using std::string;

MapLoader::MapLoader() {
    LoadAssets();
}

MapLoader::~MapLoader() {}

Map* MapLoader::LoadMap(const string& filename) {
    int width, height;
    Map* map = new Map();

    std::fstream file(filename);
    if (file.fail()) {
        cout << "Failed to open file \"" << filename << "\". Exiting." << endl;
        exit(1);
    }

    file >> width >> height;

    string line;
    std::vector<string> lines;
    while (getline(file, line)) {
        if (line.length() == 0 || line.at(0) == '#') continue;

        cout << "Row " << lines.size() << ": " << line << endl;
        if (line.length() != width) {
            cout << "Row " << lines.size() << " of map had incorrect width of " << line.length() << endl;
            exit(1);
        }

        lines.push_back(line);
    }

    if (lines.size() != height) {
        cout << "Map had incorrect height of " << lines.size() << endl;
        exit(1);
    }

    if (!wall_model_ || !door_model_ || !key_model_) {
        cout << "Models failed to initialize for map. Exiting..." << endl;
        exit(1);
    }

    bool add_ground = false;
    GameObject* current_object;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            char current_char = lines[j][i];
            glm::vec3 base_position = GetPositionForCoordinate(i, j);
            if (IsKey(current_char)) {
                current_object = new Key(key_model_, map, current_char, glm::vec2(base_position));
                // Key transforms itself
                add_ground = true;
            } else if (IsDoor(current_char)) {
                current_object = new Door(door_model_, current_char);
                current_object->transform->Translate(base_position);
                add_ground = true;
            } else {
                switch (current_char) {
                    case 'W':
                        current_object = new Wall(wall_model_);
                        current_object->transform->Scale(glm::vec3(1, 1, 1.3));
                        current_object->transform->Translate(base_position);
                        current_object->SetTextureIndex(TEX0);
                        break;
                    case 'S':
                        current_object = new Spawn(start_model_);
                        current_object->transform->Translate(glm::vec3(base_position.x, base_position.y, 0));
                        current_object->transform->Scale(0.2f);
                        add_ground = true;
                        break;
                    case 'G':
                        current_object = new Goal(goal_model_, map);
                        current_object->transform->Translate(base_position);
                        current_object->transform->Rotate(0.1, glm::vec3(0, 0, 1));
                        add_ground = true;
                        break;
                    case '0':
                        current_object = GetGround(base_position);
                        break;
                    default:
                        printf("Unrecognized character \'%c\'", current_char);
                        continue;
                }
            }

            current_object->material = GetMaterialForCharacter(current_char);
            map->Add(current_object);

            if (add_ground) {
                current_object = GetGround(base_position);
                current_object->material = GetMaterialForCharacter(current_char);
                map->Add(current_object);
                add_ground = false;
            }

            // Add ceiling
            current_object = GetGround(base_position);
            current_object->transform->Translate(glm::vec3(0, 0, 2.3));
            current_object->material = GetMaterialForCharacter(current_char);
            map->Add(current_object);
        }
    }

    return map;
}

Material MapLoader::GetMaterialForCharacter(char c) {
    switch (std::tolower(c)) {
        case 'a':
            return Material(1, 0.9f, 0);
        case 'b':
            return Material(0.3f, 0.5f, 0.8f);
        case 'c':
            return Material(0.5f, 0.7f, 0.1f);
        case 'd':
            return Material(0.1f, 0.2f, 0.1f);
        case 'e':
            return Material(0.8f, 0.2f, 0.8f);
        case 's':
            return Material(0, 1, 0);
        case 'g':
            return Material(1, 1, 0);
        default:
            return Material();
    }
}

void MapLoader::LoadAssets() {
    wall_model_ = new Model("models/cube.txt");
    door_model_ = new Model("models/knot.txt");
    key_model_ = new Model("models/mjolnir.obj");
    start_model_ = new Model("models/sphere.txt");
    goal_model_ = new Model("models/goal_crystal.obj");
}

GameObject* MapLoader::GetGround(glm::vec3 base_position) const {
    GameObject* ground = new Wall(wall_model_, false);
    ground->transform->Translate(glm::vec3(base_position.x, base_position.y, -0.5));
    ground->SetTextureIndex(TEX1);

    return ground;
}

glm::vec3 MapLoader::GetPositionForCoordinate(int i, int j) {
    return glm::vec3(i, j, 0) + glm::vec3(0.5) + glm::vec3(0, 0, GROUND_LEVEL);
}

bool MapLoader::IsDoor(char c) {
    return c >= 65 && c <= 69;
}

bool MapLoader::IsKey(char c) {
    return c >= 97 && c <= 101;
}
