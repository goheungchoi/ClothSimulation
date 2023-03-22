// Copyright 2016 University of Minnesota
// 
// TRIMESH Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef TRIMESH_HPP
#define TRIMESH_HPP 1

#include "vector.hpp"
#include "matrix.hpp"
#include "shader.hpp"
#include "stb_image.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>

//
//	Triangle Mesh Class
//
class TriMesh {
public:
	GLuint verts_vbo, colors_vbo, normals_vbo, uvs_vbo,
		faces_ibo, tris_vao;

	GLuint texture;

	std::vector<Vec3f> vertices;
	std::vector<Vec3f> normals;
	std::vector<Vec3f> colors;
	std::vector<Vec3i> faces;
	std::vector<Vec2f> uvs;

	Vec3f scalingVec;
	Vec3f translatingVec;
	Mat4x4 rotationMat;

	TriMesh();
	TriMesh(std::string file);

	// Set Buffers
	void initBuffers();
	void bindElementBuffers();
	void unbindElementBuffers();

	// Create texture
	void bindTexture(std::string file);

	// Draw object
	void draw(mcl::Shader& shader);

	// Compute normals if not loaded from obj
	// or if recompute is set to true.
	void need_normals( bool recompute=false );

	// Sets a default vertex color if
	// they haven't been set.
	void set_colors( Vec3f default_color = Vec3f(0.4,0.4,0.4) );

	// Loads an OBJ file
	bool load_obj( std::string file );

	// Prints details about the mesh
	void print_details();
	void print_AABB_size();

	void scale(float alpha);
	void scale(float x, float y, float z);
	void translate(float x, float y, float z);
	void rotate(float xtheta, float ytheta, float ztheta);
	Mat4x4 get_model_mat();

private:
	Mat4x4 model_mat;
};

static void split_str(char delim, const std::string& str, std::vector<std::string>* result);

#endif