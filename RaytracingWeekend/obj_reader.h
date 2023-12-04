#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <regex>
#include "rtweekend.h"
#include "triangle.h"
#include "hittable.h"
#include "hittable_list.h"

hittable_list obj(std::string filename, std::shared_ptr<material> mat) {
    std::vector< unsigned int > vertexIndices, normalIndices;
    std::vector< vec3 > temp_vertices;
    std::vector< vec3 > temp_normals;

    hittable_list my_tris;

    std::ifstream file{ filename, std::ifstream::in };
    if (!file.is_open()) {
        std::cerr << "Couldn't open the file: " << filename << std::endl;
        return my_tris;
    }

    std::string lineHeader;
    while (file >> lineHeader) {
        if (lineHeader == "v") {
            // read the vertices of the mesh
            vec3 vertex;
            file >> vertex[0] >> vertex[1] >> vertex[2];
            temp_vertices.push_back(vertex);
        }
        else if (lineHeader == "vn") {
            // read the normals (currently not in use)
            vec3 normal;
            file >> normal[0] >> normal[1] >> normal[2];
            temp_normals.push_back(normal);
        }
        else if (lineHeader == "f") {
            //read the faces of the mesh
            unsigned int vertexIndex[3], normalIndex[3];

            //store the face data into a string, so it doesn't get consumed
            std::string strBuffer;
            std::getline(file, strBuffer);

            //pattern match for 1/2/3 (data including UV) or 1//2 (face data without UV information)
            int matches = sscanf(strBuffer.c_str(), "%d/%*d/%d %d/%*d/%d %d/%*d/%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
            if (matches != 6) {
                matches = sscanf(strBuffer.c_str(), "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
                if (matches != 6) {
                    std::cerr << "Parser couldn't read the file contents." << std::endl;
                    return my_tris;
                }
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);

            my_tris.add(make_shared<triangle>(
                temp_vertices[vertexIndex[0] - 1], 
                temp_vertices[vertexIndex[1] - 1], 
                temp_vertices[vertexIndex[2] - 1], 
                temp_normals[normalIndex[0]-1],
                mat));
        }
    }
    std::cerr << "Loaded "<<filename<<" with "<< normalIndices.size() << " normals and " << vertexIndices.size()<< " vertecies" << std::endl;
    return my_tris;
}