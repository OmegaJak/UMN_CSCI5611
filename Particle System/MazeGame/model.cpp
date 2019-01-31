#define _CRT_SECURE_NO_WARNINGS

#include <detail/type_vec3.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include "constants.h"
#include "model.h"
#include "model_manager.h"

using std::string;
using std::vector;

Model::Model(const string& file) {
    unsigned int dot_position = file.find_last_of('.');
    if (dot_position == string::npos) {
        printf("Given file \"%s\" did not have an extension. Exiting...\n", file.c_str());
        exit(1);
    }

    string extension = file.substr(dot_position + 1, 3);
    if (extension == "txt") {
        LoadTxt(file);
    } else if (extension == "obj") {
        LoadObj(file);
    } else {
        printf("Unrecognized file extension \"%s\" for file \"%s\". Exiting...\n", extension.c_str(), file.c_str());
        exit(1);
    }

    ModelManager::RegisterModel(this);
}

void Model::LoadTxt(const std::string& file) {
    std::ifstream modelFile;
    modelFile.open(file);
    int num_elements;
    modelFile >> num_elements;
    model_ = new float[num_elements];

    for (int i = 0; i < num_elements; i++) {
        modelFile >> model_[i];
    }

    printf("%d\n", num_elements);
    num_verts_ = num_elements / ELEMENTS_PER_VERT;
    modelFile.close();
}

// Credit for the basis of this methodology goes to http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// This only supports an obj defining vertex positions, normals, and triangles
void Model::LoadObj(const std::string& filename) {
    vector<unsigned int> vertex_indices, normal_indices;
    vector<glm::vec3> temp_vertices;
    vector<glm::vec3> temp_normals;

    FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
        printf("Failed to open file \"%s\". Exiting...\n", filename.c_str());
        exit(1);
    }

    while (true) {
        char line_header[128];

        // read the first word of the line
        int res = fscanf(file, "%s", line_header);
        if (res == EOF) break;  // End of file. Leave loop

        if (strcmp(line_header, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if (strcmp(line_header, "vt") == 0) {
            glm::vec3 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
        } else if (strcmp(line_header, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if (strcmp(line_header, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertex_index[3], normal_index[3];
            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertex_index[0], &normal_index[0], &vertex_index[1], &normal_index[1],
                                 &vertex_index[2], &normal_index[2]);
            if (matches != 6) {
                printf("File \"%s\" can't be read by this simple parser. Exiting...\n", filename.c_str());
                exit(1);
            }

            vertex_indices.push_back(vertex_index[0]);
            vertex_indices.push_back(vertex_index[1]);
            vertex_indices.push_back(vertex_index[2]);
            normal_indices.push_back(normal_index[0]);
            normal_indices.push_back(normal_index[1]);
            normal_indices.push_back(normal_index[2]);
        }
    }

    // We've parsed the data into vectors, so now transform the vector data into OpenGL data
    if (vertex_indices.size() % 3 != 0) {
        printf("The number of vertex indices was not a factor of 3. Exiting...");
        exit(1);
    }

    int num_verts = vertex_indices.size();
    int model_size = num_verts * ELEMENTS_PER_VERT;
    model_ = new float[model_size];
    for (int vertex_number = 0; vertex_number < num_verts; vertex_number++) {
        int vertex_index = vertex_indices[vertex_number];
        glm::vec3 vertex = temp_vertices[vertex_index - 1];  // subtract 1 because objs are indexed from 1, not 0

        int vertex_offset = vertex_number * ELEMENTS_PER_VERT + POSITION_OFFSET;
        model_[vertex_offset] = vertex.x;
        model_[vertex_offset + 1] = vertex.y;
        model_[vertex_offset + 2] = vertex.z;

        int normal_index = normal_indices[vertex_number];
        glm::vec3 normal = temp_normals[normal_index - 1];  // subtract 1 because objs are indexed from 1

        int normal_offset = vertex_number * ELEMENTS_PER_VERT + NORMAL_OFFSET;
        model_[normal_offset] = normal.x;
        model_[normal_offset + 1] = normal.y;
        model_[normal_offset + 2] = normal.z;

        int uv_offset = vertex_number * ELEMENTS_PER_VERT + TEXCOORD_OFFSET;
        model_[uv_offset] = model_[uv_offset + 1] = 0;  // Set uv's to zero because this parser doesn't handle them and they're expected
    }

    num_verts_ = num_verts;
    fclose(file);
}

int Model::NumElements() const {
    return num_verts_ * ELEMENTS_PER_VERT;
}

int Model::NumVerts() const {
    return num_verts_;
}

std::vector<glm::vec4> Model::Vertices() const {
    std::vector<glm::vec4> verts;
    for (int i = POSITION_OFFSET; i < NumElements(); i += ATTRIBUTE_STRIDE) {
        verts.push_back(glm::vec4(model_[i], model_[i + 1], model_[i + 2], 1.0));  // These are positions so w=1 for vec4s
    }

    return verts;
}
