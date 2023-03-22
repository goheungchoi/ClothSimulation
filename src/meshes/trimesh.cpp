#include "trimesh.hpp"

TriMesh::TriMesh() {
	texture = 0;	// texture is disabled at first
	scalingVec[0] = 1.f; scalingVec[1] = 1.f; scalingVec[2] = 1.f;
	translatingVec[0] = 0.f; translatingVec[1] = 0.f; translatingVec[2] = 0.f;
}

TriMesh::TriMesh(std::string file) {
	texture = 0;	// texture is disabled at first
	scalingVec[0] = 1.f; scalingVec[1] = 1.f; scalingVec[2] = 1.f;
	translatingVec[0] = 0.f; translatingVec[1] = 0.f; translatingVec[2] = 0.f;
	load_obj(file);
}

void TriMesh::initBuffers() {
	// Create the buffer for vertices
	glGenBuffers(1, &verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo);

	// Create the buffer for normals
	glGenBuffers(1, &normals_vbo);

	// Create the buffer for colors
	glGenBuffers(1, &colors_vbo);

	// Create the buffer for uvs
	glGenBuffers(1, &uvs_vbo);

	// Create the buffer for indices
	glGenBuffers(1, &faces_ibo);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
}

void TriMesh::bindTexture(std::string filename) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	std::cout << "Texture loading success" << std::endl;
}

void TriMesh::bindElementBuffers() {
	// Bind buffers
	glBindVertexArray(tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(faces[0]), &faces[0][0], GL_STATIC_DRAW);
}

void TriMesh::unbindElementBuffers() {
	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void TriMesh::draw(mcl::Shader& shader) {
	// Send updated info to the GPU
	glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, get_model_mat().m); // model transformation

	bindElementBuffers();

	// bind vbo
	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0][0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), 0);

	// location=1 is the normal
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0][0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(normals[0]), 0);

	// location=2 is the color
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), &colors[0][0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(colors[0]), 0);

	// location=3 is the uv
	if (texture) {
		// Active the texture in the shader
		glUniform1i(shader.uniform("useTexture"), 1);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, uvs_vbo);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(uvs[0]), &uvs[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, vert_dim, GL_FLOAT, GL_FALSE, sizeof(uvs[0]), 0);
	}
	else {
		// Disable the texture use in the shader
		glUniform1i(shader.uniform("useTexture"), 0);
		glDisableVertexAttribArray(3);
	}
	// Done setting data for the vao
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
	unbindElementBuffers();
}

void TriMesh::print_details() {
	std::cout << "--- Detail --- " << std::endl;
	std::cout << "Vertices: " << vertices.size() << std::endl;
	std::cout << "Normals: " << normals.size() << std::endl;
	std::cout << "Colors: " << colors.size() << std::endl;
	std::cout << "Faces: " << faces.size() << std::endl;
	std::cout << "UVs: " << uvs.size() << std::endl;
	if (texture) {
		std::cout << "Texture loaded" << std::endl;
	}
	std::cout << std::endl;
}

void TriMesh::print_AABB_size() {
	float max_x = vertices[0][0];
	float min_x = vertices[0][0];
	float max_y = vertices[0][1];
	float min_y = vertices[0][1];
	float max_z = vertices[0][2];
	float min_z = vertices[0][2];

	for (Vec3f& v : vertices) {
		Vec3f newv = get_model_mat() * v;

		float x = newv[0]; float y = newv[1]; float z = newv[2];

		if (max_x < x) max_x = x;
		if (min_x > x) min_x = x;
		if (max_y < y) max_y = y;
		if (min_y > y) min_y = y;
		if (max_z < z) max_z = z;
		if (min_z > z) min_z = z;
	}

	std::cout << "x-axis: (" << min_x << ", " << max_x << ")" << std::endl;
	std::cout << "y-axis: (" << min_y << ", " << max_y << ")" << std::endl;
	std::cout << "z-axis: (" << min_z << ", " << max_z << ")" << std::endl;
	std::cout << std::endl;
}

void TriMesh::need_normals(bool recompute) {
	if (vertices.size() == normals.size() && !recompute) { return; }
	if (normals.size() != vertices.size()) { normals.resize(vertices.size()); }
	std::cout << "Computing TriMesh normals..." << std::endl;
	const int nv = normals.size();
	for (int i = 0; i < nv; ++i) { normals[i][0] = 0.f; normals[i][1] = 0.f; normals[i][2] = 0.f; }
	int nf = faces.size();
	for (int f = 0; f < nf; ++f) {
		Vec3i face = faces[f];
		const Vec3f& p0 = vertices[face[0]];
		const Vec3f& p1 = vertices[face[1]];
		const Vec3f& p2 = vertices[face[2]];
		Vec3f a = p0 - p1, b = p1 - p2, c = p2 - p0;
		float l2a = a.len2(), l2b = b.len2(), l2c = c.len2();
		if (!l2a || !l2b || !l2c) { continue; } // check for zeros or nans
		Vec3f facenormal = a.cross(b);
		normals[faces[f][0]] += facenormal * (1.0f / (l2a * l2c));
		normals[faces[f][1]] += facenormal * (1.0f / (l2b * l2a));
		normals[faces[f][2]] += facenormal * (1.0f / (l2c * l2b));
	}
	for (int i = 0; i < nv; i++) { normals[i].normalize(); }
} // end need normals

void TriMesh::set_colors(Vec3f default_color) {
	colors.assign(vertices.size(), default_color);
} // end need colors

// Function to split a string into multiple strings, seperated by delimeter
static void split_str(char delim, const std::string& str, std::vector<std::string>* result) {
	std::stringstream ss(str); std::string s;
	while (std::getline(ss, s, delim)) { result->push_back(s); }
}

bool TriMesh::load_obj(std::string file) {

	std::cout << "\nLoading " << file << std::endl;

	//	README:
	//
	//	The problem with standard obj files and opengl is that
	//	there isn't a good way to make triangles with different indices
	//	for vertices/normals. At least, not any way that I'm aware of.
	//	So for now, we'll do the inefficient (but robust) way:
	//	redundant vertices/normals.
	//

	std::vector<Vec3f> temp_normals;
	std::vector<Vec3f> temp_verts;
	std::vector<Vec3f> temp_colors;

	//
	//	First loop, make buffers
	//
	std::ifstream infile(file.c_str());
	if (infile.is_open()) {

		std::string line;
		while (std::getline(infile, line)) {

			std::stringstream ss(line);
			std::string tok; ss >> tok;

			// Vertex
			if (tok == "v") {

				// First three location
				float x, y, z; ss >> x >> y >> z;
				temp_verts.push_back(Vec3f(x, y, z));

				// Next three colors
				float cx, cy, cz;
				if (ss >> cx >> cy >> cz) {
					temp_colors.push_back(Vec3f(cx, cy, cz));
				}
				else {
					temp_colors.push_back(Vec3f(0.3f, 0.3f, 0.3f));
				}
			}

			// Normal
			if (tok == "vn") {
				float x, y, z; ss >> x >> y >> z;
				temp_normals.push_back(Vec3f(x, y, z));
			}

		} // end loop lines

	} // end load obj
	else { std::cerr << "\n**TriMesh::load_obj Error: Could not open file " << file << std::endl; return false; }

	//
	//	Second loop, make faces
	//
	std::ifstream infile2(file.c_str());
	if (infile2.is_open()) {

		std::string line;
		while (std::getline(infile2, line)) {

			std::stringstream ss(line);
			std::string tok; ss >> tok;

			// Face
			if (tok == "f") {

				Vec3i face;
				// Get the three vertices
				for (size_t i = 0; i < 3; ++i) {

					std::string f_str; ss >> f_str;
					std::vector<std::string> f_vals;
					split_str('/', f_str, &f_vals);
					assert(f_vals.size() > 0);

					face[i] = vertices.size();
					int v_idx = std::stoi(f_vals[0]) - 1;
					vertices.push_back(temp_verts[v_idx]);
					colors.push_back(temp_colors[v_idx]);

					// Check for normal
					if (f_vals.size() > 2) {
						int n_idx = std::stoi(f_vals[2]) - 1;
						normals.push_back(temp_normals[n_idx]);
					}
				}

				faces.push_back(face);

				// If it's a quad, make another triangle
				std::string last_vert = "";
				if (ss >> last_vert) {
					Vec3i face2;
					face2[0] = face[0];
					face2[1] = face[2];

					std::vector<std::string> f_vals;
					split_str('/', last_vert, &f_vals);
					assert(f_vals.size() > 0);

					int v_idx = std::stoi(f_vals[0]) - 1;
					vertices.push_back(temp_verts[v_idx]);
					colors.push_back(temp_colors[v_idx]);
					face2[2] = vertices.size();

					// Check for normal
					if (f_vals.size() > 2) {
						int n_idx = std::stoi(f_vals[2]) - 1;
						normals.push_back(temp_normals[n_idx]);
					}

					faces.push_back(face2);
				}

			} // end parse face

		} // end loop lines

	} // end load obj

	// Make sure we have normals
	if (!normals.size()) {
		std::cout << "**Warning: normals not loaded so we'll compute them instead." << std::endl;
		need_normals();
	}

	return true;

} // end load obj

void TriMesh::scale(float alpha) {
	scalingVec[0] = alpha;
	scalingVec[1] = alpha;
	scalingVec[2] = alpha;
}

void TriMesh::scale(float x, float y, float z) {
	scalingVec[0] = x;
	scalingVec[1] = y;
	scalingVec[2] = z;
}

void TriMesh::translate(float x, float y, float z) {
	translatingVec[0] = x;
	translatingVec[1] = y;
	translatingVec[2] = z;
}

void TriMesh::rotate(float xtheta, float ytheta, float ztheta) {
	rotationMat.make_identity();
}

Mat4x4 TriMesh::get_model_mat() {
	Mat4x4 model;
	model.m[0] = scalingVec[0]; model.m[4] = 0.f;			model.m[8] = 0.f;			 model.m[12] = translatingVec[0];
	model.m[1] = 0.f;			model.m[5] = scalingVec[1]; model.m[9] = 0.f;			 model.m[13] = translatingVec[1];
	model.m[2] = 0.f;			model.m[6] = 0.f;			model.m[10] = scalingVec[2]; model.m[14] = translatingVec[2];
	model.m[3] = 0.f;			model.m[7] = 0.f;			model.m[11] = 0.f;			 model.m[15] = 1.f;

	return model;
}