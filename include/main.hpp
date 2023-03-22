#ifndef MAIN_HPP
#define MAIN_HPP

#include <CL/cl.h>
#include <CL/opencl.h>
// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Includes
#include "vector.hpp"
#include "trimesh.hpp"
#include "shader.hpp"
#include "matrix.hpp"
#include "config.hpp"
#include <cstring> // memcpy
#include <cmath>

// Constants
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

typedef struct frustum {
	float width;
	float height;
	float dNear;
	float dFar;
	float aspect;
	float vfov;
	float near;
	float far;

	void set_frustum(
		const float vfov_,
		const float aspect_,
		const float dNear_,
		const float dFar_) {
		vfov = vfov_;
		aspect = aspect_;
		dNear = dNear_;
		dFar = dFar_;
		// Set height and width
		height = 2 * dNear * tan((vfov / 2.0) * PI / 180.0);
		width = height * aspect;

		near = dNear;
		far = dFar;
	}

	Mat4x4 get_projection_mat() {
		Mat4x4 proj;
		proj.m[0] = (1.f / tan((vfov / 2.f) * PI / 180.f)) / aspect;
		proj.m[1] = 0.f;
		proj.m[2] = 0.f;
		proj.m[3] = 0.f;
		//
		proj.m[4] = 0.f;
		proj.m[5] = (1.f / tan((vfov / 2.f) * PI / 180.f));
		proj.m[6] = 0.f;
		proj.m[7] = 0.f;
		//
		proj.m[8] = 0.f;
		proj.m[9] = 0.f;
		proj.m[10] = -(far - near) / (far + near);
		proj.m[11] = -1.f;
		//
		proj.m[12] = 0.f;
		proj.m[13] = 0.f;
		proj.m[14] = -2.f * far * near / (far - near);
		proj.m[15] = 0.f;
		return proj;
	}
} Frustum;
bool pause = true;
//	Global state variables
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height; // window size
	float aspect;
	//GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	std::vector<TriMesh> meshes;
	std::vector<unsigned int> cloth_pins;

	Frustum frus;
	Vec3f n;
	Vec3f u;
	Vec3f v;
	Vec3f eye;
	Vec3f view_dir;
	Vec3f up_dir;
	//  Model, view and projection matrices, initialized to the identity
	Mat4x4 model;
	Mat4x4 R;
	Mat4x4 T;
	Mat4x4 view;
	Mat4x4 projection;
}

// Kernal variables
namespace Kernel {
	std::vector<cl_device_id> devices;
	cl_program program;
	cl_context context;
	cl_command_queue commandQueue;

	std::vector<cl_float3> pos;
	std::vector<cl_float3> n;
	cl_mem old_positions;
	cl_mem positions;
	cl_mem new_positions;
	cl_mem normals;
	
	cl_kernel updatePositionKernel;
	cl_kernel updateOldPositionKernel;
	cl_kernel constraintOddKernel;
	cl_kernel constraintEvenKernel;
	cl_kernel calculateNoramlsKernel;
}

// Function to set up geometry
void init_meshes();
void move_pins(int key);
float cl_float3_dist(cl_float3& v1, cl_float3& v2);
// Functions to set up kernels
void init_kernel();
cl_program build_prog(std::string& filename);
void release_kernel();
void set_buffer_kernel();
void execute_kernel();
void get_result_from_kernel();
void clSetKernelArgAssert(cl_int err);
void clCreateKernelAssert(cl_int err);
void clEnqueueNDRangeKernelAssert(cl_int err);
// Functions to set up transformation matrices
void init_mat();
void set_view_mat();
void set_projection_mat();

// Callback functions
static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#endif