#pragma once
#include <stdio.h>
#include <regex>
#include "rtweekend.h"
#include "triangle.h"
#include "hittable.h"
#include "hittable_list.h"

hittable_list obj(const char * filename, std::shared_ptr<material> mat) {
    std::vector< unsigned int > vertexIndices, normalIndices;
    std::vector< vec3 > temp_vertices;
    std::vector< vec3 > temp_normals;

    hittable_list my_tris;

    FILE* file;
    fopen_s(&file, filename, "r");

    if (file == NULL) {
        std::cerr << "Couldn't open the file: " << filename << std::endl;
        return my_tris;
    }

    while (1) {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf_s(file, "%s", lineHeader);
        if (res == EOF) {
            break; // EOF = End Of File. Quit the loop.
        }

        if (strcmp(lineHeader, "v") == 0) {
            //read the vertecies of the mesh
            vec3 vertex;
            fscanf_s(file, "%lf %lf %lf\n", &vertex[0], &vertex[1], &vertex[2]);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            //read the normals (currently not in use)
            vec3 normal;
            fscanf_s(file, "%lf %lf %lf\n", &normal[0], &normal[1], &normal[2]);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            //read the faces of the mesh
            unsigned int vertexIndex[3], normalIndex[3];

            //store the face data into a string, so it doesn't get consumed
            char strBuffer[256];
            auto line = fgets(strBuffer, 256, file);

            //pattern match for 1/2/3 (data including UV) or 1//2 (face data without UV information)
            int matches = sscanf_s(strBuffer, "%d/%*d/%d %d/%*d/%d %d/%*d/%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
            if (matches != 6) {
                matches = sscanf_s(strBuffer, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
                if (matches != 6) {
                    std::cerr << "Parser couldn't read the file contents." << std::endl;
                    return my_tris;
                }
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            my_tris.add(make_shared<triangle>(
                temp_vertices[vertexIndex[0] - 1], 
                temp_vertices[vertexIndex[1] - 1], 
                temp_vertices[vertexIndex[2] - 1], 
                mat));
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    std::cerr << "Loaded Mesh with "<< my_tris.objects.size() << " triangles and " << vertexIndices.size()<< " vertecies" << std::endl;
    return my_tris;
}