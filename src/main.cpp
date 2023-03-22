#include "main.hpp"

/* Main */	
int main(int argc, char *argv[]){
	// Load the meshes
	init_meshes();
	
	// Set up the window variable
	GLFWwindow* window;
    
    // Define the error callback function
	glfwSetErrorCallback(&error_callback);

	// Initialize glfw
	if( !glfwInit() ){ return EXIT_FAILURE; }

	// Ask for OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	Globals::win_width = WIN_WIDTH;
	Globals::win_height = WIN_HEIGHT;
	window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "ClothSimulation", NULL, NULL);
	if( !window ){ glfwTerminate(); return EXIT_FAILURE; }

	// Define callbacks to handle user input and window resizing
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// More setup stuff
	glfwMakeContextCurrent(window); // Make the window current
    glfwSwapInterval(1); // Set the swap interval

	// make sure the openGL and GLFW code can be found
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Initialize the shaders
	// MY_SRC_DIR was defined in CMakeLists.txt
	// it specifies the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss; ss << MY_CUR_DIR << "shaders/shader.";
	shader.init_from_files( ss.str()+"vert", ss.str()+"frag" );

	// Initialize the OpenGL buffers
	for (TriMesh& mesh : Globals::meshes)
		mesh.initBuffers();

	// binds texture to the cloth
	std::stringstream texture_file; texture_file << MY_DATA_DIR << "textures/cloth_texture.jpg";
	Globals::meshes[0].bindTexture(texture_file.str());

	// set frame buffer
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Perform some OpenGL initializations
	glEnable(GL_DEPTH_TEST);  // turn hidden surfce removal on
	glClearColor(1.f,1.f,1.f,1.f);  // set the background to white

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();
    
	/* OpenCL initialization steps */
	init_kernel();
	set_buffer_kernel();

	// Initialize matrices
	init_mat();

	// Print Meshes details
	for (TriMesh& mesh : Globals::meshes) {
		mesh.print_details();
		mesh.print_AABB_size();
	}

	// Game loop
	while( !glfwWindowShouldClose(window) ){
		
		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send updated info to the GPU
		glUniformMatrix4fv( shader.uniform("model"), 1, GL_FALSE, Globals::model.m  ); // model transformation
		glUniformMatrix4fv( shader.uniform("view"), 1, GL_FALSE, Globals::view.m  ); // viewing transformation
		glUniformMatrix4fv( shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m ); // projection matrix

		// Draw
		for (TriMesh& mesh : Globals::meshes)
			mesh.draw(shader);

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (pause) continue;
		// Calculate the cloth physics
		execute_kernel();
		get_result_from_kernel();
	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();
    
	// Release kernels
	release_kernel();

	return EXIT_SUCCESS;
}

void init_kernel() {
	cl_int err;
	cl_platform_id platform;
	cl_uint platform_amount;

	/* Get platform */
	err = clGetPlatformIDs(1, &platform, &platform_amount);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: platform not found!" << std::endl;
		exit(1);
	}

	char platform_name[128];
	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(char)*128, platform_name, NULL);
	if (err != CL_SUCCESS) {
		if (err == CL_INVALID_PLATFORM) 
			std::cout << "ERROR: invalid platform!" << std::endl;
		else if(err == CL_INVALID_VALUE)
			std::cout << "ERROR: invalid value!" << std::endl;
		else 
			std::cout << "ERROR: unknown error occured!" << std::endl;
		exit(1);
	}
	std::cout << "SUCCESS: platform found... (" << platform_name << ")" << std::endl;

	/* Get devices*/
	cl_uint devices_amount = 0;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &devices_amount);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: device not found!" << std::endl;
		exit(1);
	}

	Kernel::devices.resize(devices_amount);
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, devices_amount, &Kernel::devices[0], 0);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: device not found!" << std::endl;
		exit(1);
	}

	char device_name[128];
	err = clGetDeviceInfo(Kernel::devices[0], CL_DEVICE_NAME, sizeof(char) * 128, device_name, NULL);
	if (err != CL_SUCCESS) {
		if (err == CL_INVALID_DEVICE)
			std::cout << "ERROR: invalid device!" << std::endl;
		else if (err == CL_INVALID_VALUE)
			std::cout << "ERROR: invalid value!" << std::endl;
		else
			std::cout << "ERROR: unknown error occured!" << std::endl;
		exit(1);
	}
	std::cout << "SUCCESS: " << devices_amount << " device(s) found... (" << device_name << ")" << std::endl;

	/* Create contents */
	Kernel::context = clCreateContext(0, devices_amount, &Kernel::devices[0], NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: creating context failed!" << std::endl;
		exit(1);
	}
	std::cout << "SUCCESS: context created..." << std::endl;

	/* Create command queue*/
	Kernel::commandQueue = clCreateCommandQueue(Kernel::context, Kernel::devices[0], NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: creating command queue failed!" << std::endl;
		exit(1);
	}
	std::cout << "SUCCESS: command queue created on the device..." << std::endl;

	// create a program
	std::stringstream kernel_file; kernel_file << MY_CUR_DIR << "kernels/kernels.cl";
	Kernel::program = build_prog(kernel_file.str());

	// create kernels
	Kernel::updatePositionKernel = clCreateKernel(Kernel::program, "update_position", &err);
	clCreateKernelAssert(err);
	Kernel::updateOldPositionKernel = clCreateKernel(Kernel::program, "update_old_position", &err);
	clCreateKernelAssert(err);
	Kernel::constraintOddKernel = clCreateKernel(Kernel::program, "constraint", &err);
	clCreateKernelAssert(err);
	Kernel::constraintEvenKernel = clCreateKernel(Kernel::program, "constraint", &err);
	clCreateKernelAssert(err);
	Kernel::calculateNoramlsKernel = clCreateKernel(Kernel::program, "calculate_normals", &err);
	clCreateKernelAssert(err);

	std::cout << "OpenCL setup is done!" << std::endl;
}

cl_program build_prog(std::string& filename) {
	cl_program program;

	std::ifstream file(filename.c_str());
	std::string lines = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	std::cout << lines << std::endl;
	file.close();

	cl_int err;
	size_t size = cl_uint(lines.size());
	const char* start = &lines[0];
	program = clCreateProgramWithSource(Kernel::context, 1, (const char**)&start, (const size_t*)&size, &err);
	if (err != CL_SUCCESS) {
		std::cout << "ERROR: Program creation failed!" << std::endl;
		exit(1);
	}

	std::stringstream dir; dir << "-I " << MY_CUR_DIR << "kernels/";
	std::string options = dir.str() + " -cl-denorms-are-zero -cl-strict-aliasing -cl-fast-relaxed-math -cl-mad-enable -cl-no-signed-zeros";
	err = clBuildProgram(program, 1, &Kernel::devices[0], options.c_str(), NULL, NULL);

	if (err != CL_SUCCESS) {
		std::cout << "ERROR: ";
		switch (err) {
		case CL_INVALID_PROGRAM:
			std::cout << "invalid program" << std::endl;
			break;
		case CL_INVALID_VALUE:
			std::cout << "invalid value" << std::endl;
			break;
		case CL_INVALID_DEVICE:
			std::cout << "invalid device" << std::endl;
			break;
		case CL_INVALID_BINARY:
			std::cout << "invalid binary" << std::endl;
			break;
		case CL_INVALID_BUILD_OPTIONS:
			std::cout << "invalid build options" << std::endl;
			break;
		case CL_INVALID_OPERATION:
			std::cout << "invalid operation" << std::endl;
			break;
		case CL_COMPILER_NOT_AVAILABLE:
			std::cout << "compiler not available" << std::endl;
			break;
		case CL_BUILD_PROGRAM_FAILURE:
			std::cout << "build program failure" << std::endl;
			break;
		case CL_OUT_OF_RESOURCES:
			std::cout << "out of resources" << std::endl;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			std::cout << "out of host memory" << std::endl;
			break;
		}

		if (err == CL_BUILD_PROGRAM_FAILURE) {
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(program, Kernel::devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char* log = (char*)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(program, Kernel::devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			printf("%s\n", log);
		}

		std::cout << "clProgram build failed!" << std::endl;
		exit(1);
	}

	std::cout << "SUCCESS: clProgram built..." << std::endl;
	return program;
}

void release_kernel() {
	// Clean up
	cl_int err;
	err = clFlush(Kernel::commandQueue);
	err = clFinish(Kernel::commandQueue);
	err = clReleaseKernel(Kernel::updatePositionKernel);
	err = clReleaseKernel(Kernel::updateOldPositionKernel);
	err = clReleaseKernel(Kernel::constraintOddKernel);
	err = clReleaseKernel(Kernel::constraintEvenKernel);
	err = clReleaseKernel(Kernel::calculateNoramlsKernel);
	err = clReleaseProgram(Kernel::program);
	err = clReleaseMemObject(Kernel::old_positions);
	err = clReleaseMemObject(Kernel::positions);
	err = clReleaseMemObject(Kernel::new_positions);
	err = clReleaseMemObject(Kernel::normals);
	err = clReleaseCommandQueue(Kernel::commandQueue);
	err = clReleaseContext(Kernel::context);
}

void set_buffer_kernel() {
	TriMesh* fabric = &Globals::meshes[0];

	// Cast type from float[3] to cl_float3 (sizeof(cl_float3) == sizeof(cl_float4))
	for (Vec3f &v : fabric->vertices) {
		cl_float3 pos;
		pos.x = v[0]; pos.y = v[1]; pos.z = v[2];
		//std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		Kernel::pos.push_back(pos);
	}
	for (Vec3f &vn : fabric->normals) {
		cl_float3 n;
		n.x = vn[0]; n.y = vn[1]; n.z = vn[2];
		Kernel::n.push_back(n);
	}

	std::cout << "Data size verification..." << std::endl;
	std::cout << "Size of host data mem: " << sizeof(Kernel::pos[0]) << std::endl;
	std::cout << "Size of kernel data mem: " << sizeof(cl_float3) << std::endl;
	if (sizeof(Kernel::pos[0]) != sizeof(cl_float3)) {
		std::cout << "TEST1: FALSE" << std::endl;
		exit(1);
	}
	std::cout << "TEST1: TRUE" << std::endl;

	std::cout << "Size of host data mem: " << sizeof(Kernel::n[0]) << std::endl;
	std::cout << "Size of kernel data mem: " << sizeof(cl_float3) << std::endl;
	if (sizeof(Kernel::n[0]) != sizeof(cl_float3)) {
		std::cout << "TEST2: FALSE" << std::endl;
		exit(1);
	}
	std::cout << "TEST2: TRUE" << std::endl;

	cl_int err;
	Kernel::old_positions = clCreateBuffer(
		Kernel::context, CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float3) * Kernel::pos.size(), &Kernel::pos[0], &err);
	assert(!err);
	Kernel::positions = clCreateBuffer(
		Kernel::context, CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float3) * Kernel::pos.size(), &Kernel::pos[0], &err);
	assert(!err);
	Kernel::new_positions = clCreateBuffer(
		Kernel::context, CL_MEM_WRITE_ONLY,
		sizeof(cl_float3) * Kernel::pos.size(), NULL, &err);
	assert(!err);
	Kernel::normals = clCreateBuffer(
		Kernel::context, CL_MEM_WRITE_ONLY,
		sizeof(cl_float3) * Kernel::n.size(), NULL, &err);
	assert(!err);

	err = clSetKernelArg(Kernel::updatePositionKernel, 0, sizeof(cl_mem), &Kernel::old_positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::updatePositionKernel, 1, sizeof(cl_mem), &Kernel::positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::updatePositionKernel, 2, sizeof(cl_mem), &Kernel::new_positions);
	clSetKernelArgAssert(err);

	err = clSetKernelArg(Kernel::updateOldPositionKernel, 0, sizeof(cl_mem), &Kernel::old_positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::updateOldPositionKernel, 1, sizeof(cl_mem), &Kernel::positions);
	clSetKernelArgAssert(err);

	err = clSetKernelArg(Kernel::constraintOddKernel, 0, sizeof(cl_mem), &Kernel::positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::constraintOddKernel, 1, sizeof(cl_mem), &Kernel::new_positions);
	clSetKernelArgAssert(err);

	err = clSetKernelArg(Kernel::constraintEvenKernel, 0, sizeof(cl_mem), &Kernel::new_positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::constraintEvenKernel, 1, sizeof(cl_mem), &Kernel::positions);
	clSetKernelArgAssert(err);

	err = clSetKernelArg(Kernel::calculateNoramlsKernel, 0, sizeof(cl_mem), &Kernel::positions);
	clSetKernelArgAssert(err);
	err = clSetKernelArg(Kernel::calculateNoramlsKernel, 1, sizeof(cl_mem), &Kernel::normals);
	clSetKernelArgAssert(err);

	std::cout << "SUCCESS: Buffers and kernels setting is done...\n" << std::endl;
}

void clSetKernelArgAssert(cl_int err) {
	if (err == CL_SUCCESS) return;

	std::cout << "ERROR: ";
	switch (err) {
	case CL_INVALID_KERNEL:
		std::cout << "invalid kernel" << std::endl;
		exit(1);
	case CL_INVALID_ARG_INDEX:
		std::cout << "invalid arg index" << std::endl;
		exit(1);
	case CL_INVALID_ARG_VALUE:
		std::cout << "invalid arg value" << std::endl;
		exit(1);
	case CL_INVALID_MEM_OBJECT:
		std::cout << "invalid mem object" << std::endl;
		exit(1);
	case CL_INVALID_SAMPLER:
		std::cout << "invalid sampler" << std::endl;
		exit(1);
	case CL_INVALID_ARG_SIZE:
		std::cout << "invalid arg size" << std::endl;
		exit(1);
	}
}

void clCreateKernelAssert(cl_int err) {
	if (err == CL_SUCCESS) return;

	std::cout << "ERROR: ";
	switch (err) {
	case CL_INVALID_PROGRAM:
		std::cout << "invalid program" << std::endl;
		exit(1);
	case CL_INVALID_PROGRAM_EXECUTABLE:
		std::cout << "invalid program executable" << std::endl;
		exit(1);
	case CL_INVALID_KERNEL_NAME:
		std::cout << "invalid kernel name" << std::endl;
		exit(1);
	case CL_INVALID_KERNEL_DEFINITION:
		std::cout << "invalid kernel definition" << std::endl;
		exit(1);
	case CL_INVALID_VALUE:
		std::cout << "invalid value" << std::endl;
		exit(1);
	case CL_OUT_OF_HOST_MEMORY:
		std::cout << "out of host memory" << std::endl;
		exit(1);
	}
}

void clEnqueueNDRangeKernelAssert(cl_int err) {
	if (err == CL_SUCCESS) return;

	std::cout << "ERROR: ";
	switch (err) {
	case CL_INVALID_PROGRAM_EXECUTABLE:
		std::cout << "invalid program" << std::endl;
		exit(1);
	case CL_INVALID_COMMAND_QUEUE:
		std::cout << "invalid command queue" << std::endl;
		exit(1);
	case CL_INVALID_KERNEL:
		std::cout << "invalid kernel" << std::endl;
		exit(1);
	case CL_INVALID_CONTEXT:
		std::cout << "invalid context" << std::endl;
		exit(1);
	case CL_INVALID_KERNEL_ARGS:
		std::cout << "invalid kernel args" << std::endl;
		exit(1);
	case CL_INVALID_WORK_DIMENSION:
		std::cout << "invalid work dimension" << std::endl;
		exit(1);
	case CL_INVALID_WORK_GROUP_SIZE:
		std::cout << "invalid work group size" << std::endl;
		exit(1);
	case CL_INVALID_WORK_ITEM_SIZE:
		std::cout << "invalid work item size" << std::endl;
		exit(1);
	case CL_INVALID_GLOBAL_OFFSET:
		std::cout << "invalid global offset" << std::endl;
		exit(1);
	case CL_OUT_OF_RESOURCES:
		std::cout << "out of resources" << std::endl;
		exit(1);
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		std::cout << "mem object allocation failure" << std::endl;
		exit(1);
	case CL_INVALID_EVENT_WAIT_LIST:
		std::cout << "invalid event wait list" << std::endl;
		exit(1);
	case CL_OUT_OF_HOST_MEMORY:
		std::cout << "out of host memory" << std::endl;
		exit(1);
	}
}

void execute_kernel() {
	const unsigned int work_dim = 2;

	cl_int err;
	size_t globalWorkSize[work_dim] = { CLOTH_ROW+1, CLOTH_COL+1 };
	size_t localWorkSize[work_dim] = { BLOCK_SIZE, BLOCK_SIZE };

#ifdef _PINNED
	err = clEnqueueWriteBuffer(Kernel::commandQueue, Kernel::positions, CL_FALSE, sizeof(cl_float3)*0, sizeof(cl_float3), &Kernel::pos[0], 0, NULL, NULL);
	err = clEnqueueWriteBuffer(Kernel::commandQueue, Kernel::positions, CL_FALSE, sizeof(cl_float3)*4, sizeof(cl_float3), &Kernel::pos[4], 0, NULL, NULL);
	err = clEnqueueWriteBuffer(Kernel::commandQueue, Kernel::positions, CL_FALSE, sizeof(cl_float3)*9, sizeof(cl_float3), &Kernel::pos[9], 0, NULL, NULL);
	err = clEnqueueWriteBuffer(Kernel::commandQueue, Kernel::positions, CL_FALSE, sizeof(cl_float3)*14, sizeof(cl_float3), &Kernel::pos[14], 0, NULL, NULL);
	err = clEnqueueWriteBuffer(Kernel::commandQueue, Kernel::positions, CL_FALSE, sizeof(cl_float3)*19, sizeof(cl_float3), &Kernel::pos[19], 0, NULL, NULL);
#endif

	err = clEnqueueNDRangeKernel(
		Kernel::commandQueue, Kernel::updatePositionKernel,
		work_dim, NULL, globalWorkSize, localWorkSize,
		0, NULL, NULL);
	clEnqueueNDRangeKernelAssert(err);
	err = clEnqueueNDRangeKernel(
		Kernel::commandQueue, Kernel::updateOldPositionKernel,
		work_dim, NULL, globalWorkSize, localWorkSize,
		0, NULL, NULL);
	clEnqueueNDRangeKernelAssert(err);

	assert((SOLVER_ITERATIONS % 2) == 1);
	for (int i = 0; i < SOLVER_ITERATIONS; i++)
	{
		if (i % 2 == 0) {
			err = clEnqueueNDRangeKernel(
				Kernel::commandQueue, Kernel::constraintEvenKernel,
				work_dim, NULL, globalWorkSize, localWorkSize,
				0, NULL, NULL);
			clEnqueueNDRangeKernelAssert(err);
		} else {
			err = clEnqueueNDRangeKernel(
				Kernel::commandQueue, Kernel::constraintOddKernel,
				work_dim, NULL, globalWorkSize, localWorkSize,
				0, NULL, NULL);
			clEnqueueNDRangeKernelAssert(err);
		}
	}

	err = clEnqueueNDRangeKernel(
		Kernel::commandQueue, Kernel::calculateNoramlsKernel,
		work_dim, NULL, globalWorkSize, localWorkSize,
		0, NULL, NULL);
	clEnqueueNDRangeKernelAssert(err);

	err = clFinish(Kernel::commandQueue);
	assert(!err);
}

void get_result_from_kernel() {
	TriMesh* fabric = &Globals::meshes[0];
	
	cl_int err;
	err = clEnqueueReadBuffer(
		Kernel::commandQueue, Kernel::positions, CL_FALSE, 
		0, sizeof(cl_float3) * Kernel::pos.size(), (void*)&Kernel::pos[0],
		0, NULL, NULL);
	assert(!err);

	err = clEnqueueReadBuffer(
		Kernel::commandQueue, Kernel::normals, CL_FALSE, 
		0, sizeof(cl_float3) * Kernel::n.size(), (void*)&Kernel::n[0],
		0, NULL, NULL);
	assert(!err);

	for (int i = 0; i < fabric->vertices.size(); i++) {
		//std::cout << Kernel::pos[i].x << ", " << Kernel::pos[i].y << ", " << Kernel::pos[i].z << std::endl;
		//bool pinned = false;
		/*for (unsigned int& pin : Globals::cloth_pins) {
			if (pin == i) {
				pinned = true;
				break;
			}
		}*/
		//if (pinned) continue;
		fabric->vertices[i][0] = Kernel::pos[i].x;
		fabric->vertices[i][1] = Kernel::pos[i].y;
		fabric->vertices[i][2] = Kernel::pos[i].z;
	}

	for (int i = 0; i < fabric->normals.size(); i++) {
		fabric->normals[i][0] = Kernel::n[i].x;
		fabric->normals[i][1] = Kernel::n[i].y;
		fabric->normals[i][2] = Kernel::n[i].z;
	}

	err = clFinish(Kernel::commandQueue);
	assert(!err);
}



void init_meshes() {
	// 1.fabric
	TriMesh fabric;
	float top = CLOTH_TOP;
	float w = CLOTH_WIDTH;
	float h = CLOTH_HEIGHT;
	unsigned int row = CLOTH_ROW;
	unsigned int col = CLOTH_COL;
	float x_delta = w / col;
	float y_delta = h / row;

	float x_start = -w / 2.f;
	float y_start = h / 2.f;

	// Define vertices
	for (int i = 0; i < row + 1; i++) {
		for (int j = 0; j < col + 1; j++) { // top + y_start - 0.2*y_delta*i
			fabric.vertices.push_back(Vec3f(x_start + x_delta * j, top, y_start - y_delta * i));
		}
	}

	// Define indices
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			fabric.faces.push_back(Vec3i(j + i * (col + 1), (i + 1) * (col + 1) + j, (i + 1) * (col + 1) + (j + 1)));
			fabric.faces.push_back(Vec3i(j + i * (col + 1), (i + 1) * (col + 1) + (j + 1), i * (col + 1) + (j + 1)));
		}
	}

	// Define texture coordinates
	float u_delta = 1.0f / row+1;
	float v_delta = 1.0f / col+1;
	for (int i = 0; i < row + 1; i++) {
		for (int j = 0; j < col + 1; j++) {
			fabric.uvs.push_back(Vec2f(i * u_delta, j * v_delta, 0.f));
		}
	}

	fabric.need_normals();
	fabric.set_colors(Vec3f(0.5f, 0.5f, 0.5f));
	// translates it to the center
	fabric.translate(0.f, 0.f, -10.f);

	// 2.sphere
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "models/sphere.obj";
	TriMesh sphere(obj_file.str());

	sphere.set_colors(Vec3f(0.8f, 0.f, 0.f));
	sphere.scale(5);
	sphere.translate(0.f, 0.f, -10.f);

	// stores the meshes
	Globals::meshes.push_back(fabric); // meshes[0] is always the cloth
	Globals::meshes.push_back(sphere); // meshes[1:] are objects
}

void init_mat() {
	// Initialize eye position
	Globals::eye[0] = 0.f;
	Globals::eye[1] = 0.f;
	Globals::eye[2] = 25.f;

	// Initialize view direction
	Globals::view_dir[0] = 0.f;	// x
	Globals::view_dir[1] = 0.f;	// y
	Globals::view_dir[2] = -1.f;	// z

	// Initialize up direction
	Globals::up_dir[0] = 0.f;
	Globals::up_dir[1] = 1.f;
	Globals::up_dir[2] = 0.f;

	// Initialize view mat
	set_view_mat();
	// Initialize frustrum
	Globals::frus.set_frustum(60.0, Globals::aspect, 0.1f, 100.f);
	//Initialize projection mat
	set_projection_mat();
}

void set_view_mat() {
	Globals::n = -Globals::view_dir;
	Globals::n.normalize();

	Globals::u = Globals::up_dir.cross(Globals::n);
	Globals::u.normalize();

	Globals::v = Globals::n.cross(Globals::u);
	Globals::v.normalize();

	Globals::R.m[0] = Globals::u[0];
	Globals::R.m[4] = Globals::u[1];
	Globals::R.m[8] = Globals::u[2];

	Globals::R.m[1] = Globals::v[0];
	Globals::R.m[5] = Globals::v[1];
	Globals::R.m[9] = Globals::v[2];

	Globals::R.m[2] = Globals::n[0];
	Globals::R.m[6] = Globals::n[1];
	Globals::R.m[10] = Globals::n[2];

	Globals::T.m[12] = -Globals::eye[0];
	Globals::T.m[13] = -Globals::eye[1];
	Globals::T.m[14] = -Globals::eye[2];

	Globals::view = Globals::R * Globals::T;
}

void set_projection_mat() {
	Globals::projection = Globals::frus.get_projection_mat();
}


//
//	Callbacks
//
static void
error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Close on escape
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
		//case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GL_TRUE); break;
		}
	}

	// ToDo: update the viewing transformation matrix according to key presses
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_LEFT:  // left arrow key -> eye_dir_x--
			std::cout << "LEFT_ARROW pressed" << std::endl;
			rotate(Globals::view_dir, 0.f, +5.f, 0.f);
			set_view_mat();
			break;
		case GLFW_KEY_RIGHT:  // right arrow key -> eye_dir_x++
			std::cout << "RIGHT_ARROW pressed" << std::endl;
			rotate(Globals::view_dir, 0.f, -5.f, 0.f);
			set_view_mat();
			break;
		case GLFW_KEY_DOWN:  // down arrow key -> eye_dir_y--
			std::cout << "DOWN_ARROW pressed" << std::endl;
			Globals::view_dir[1] -= 0.1;
			set_view_mat();
			break;
		case GLFW_KEY_UP:  // up arrow key -> eye_dir_y++
			std::cout << "UP_ARROW pressed" << std::endl;
			Globals::view_dir[1] += 0.1;
			set_view_mat();
			break;
		case GLFW_KEY_ENTER:  // enter key -> pause
			std::cout << "ENTER pressed" << std::endl;
			pause = !pause;
			break;
		case GLFW_KEY_W:  // w key -> move forward
			std::cout << "W pressed" << std::endl;
			Globals::eye[0] += -Globals::n[0];
			Globals::eye[1] += -Globals::n[1];
			Globals::eye[2] += -Globals::n[2];
			set_view_mat();
			break;
		case GLFW_KEY_S:  // s key -> move backward
			std::cout << "S pressed" << std::endl;
			Globals::eye[0] -= -Globals::n[0];
			Globals::eye[1] -= -Globals::n[1];
			Globals::eye[2] -= -Globals::n[2];
			set_view_mat();
			break;
		case GLFW_KEY_A:  // a key -> move to the left
			std::cout << "A pressed" << std::endl;
			Globals::eye[0] += -Globals::u[0];
			Globals::eye[1] += -Globals::u[1];
			Globals::eye[2] += -Globals::u[2];
			set_view_mat();
			break;
		case GLFW_KEY_D:  // d key -> move to the right
			std::cout << "D pressed" << std::endl;
			Globals::eye[0] -= -Globals::u[0];
			Globals::eye[1] -= -Globals::u[1];
			Globals::eye[2] -= -Globals::u[2];
			set_view_mat();
			break;
		case GLFW_KEY_E:  // [ key -> move downward
			std::cout << "[ pressed" << std::endl;
			Globals::eye[0] -= Globals::v[0];
			Globals::eye[1] -= Globals::v[1];
			Globals::eye[2] -= Globals::v[2];
			set_view_mat();
			break;
		case GLFW_KEY_Q:  // ] key -> move upward
			std::cout << "] pressed" << std::endl;
			Globals::eye[0] += Globals::v[0];
			Globals::eye[1] += Globals::v[1];
			Globals::eye[2] += Globals::v[2];
			set_view_mat();
			break;
#ifdef _PINNED
		case GLFW_KEY_LEFT_BRACKET:
			move_pins(GLFW_KEY_LEFT_BRACKET);
			break;
		case GLFW_KEY_RIGHT_BRACKET:
			move_pins(GLFW_KEY_RIGHT_BRACKET);
			break;
#endif
		default:
			break;
		}
		/*std::cout << "eye_pos: " << Globals::eye[0] << " " << Globals::eye[1] << " " << Globals::eye[2] << std::endl;
		std::cout << "view_dir: " << Globals::view_dir[0] << " " << Globals::view_dir[1] << " " << Globals::view_dir[2] << std::endl;*/
	}
}

void move_pins(int key) {
	const float move_dist = 0.1f;
	const float min_interval = 1.5f;
	const float max_interval = 5.f;
	if (key == GLFW_KEY_LEFT_BRACKET) {
		if (cl_float3_dist(Kernel::pos[0], Kernel::pos[4]) > min_interval) {
			Kernel::pos[4].x -= move_dist;
			std::cout << Kernel::pos[4].x << std::endl;
		}
		if (cl_float3_dist(Kernel::pos[4], Kernel::pos[9]) > min_interval) {
			Kernel::pos[9].x -= move_dist;
		}
		if (cl_float3_dist(Kernel::pos[9], Kernel::pos[14]) > min_interval) {
			Kernel::pos[14].x -= move_dist;
		}
		if (cl_float3_dist(Kernel::pos[14], Kernel::pos[19]) > min_interval) {
			Kernel::pos[19].x -= move_dist;
		}
		std::cout << "pins pos: " << Kernel::pos[0].x << " " << Kernel::pos[4].x << " ";
		std::cout << Kernel::pos[9].x << " " << Kernel::pos[14].x << " " << Kernel::pos[19].x << std::endl;
	} else if (key == GLFW_KEY_RIGHT_BRACKET) {
		if (cl_float3_dist(Kernel::pos[0], Kernel::pos[4]) < max_interval) {
			Kernel::pos[4].x += move_dist;
			std::cout << Kernel::pos[4].x << std::endl;
		}
		if (cl_float3_dist(Kernel::pos[4], Kernel::pos[9]) < max_interval) {
			Kernel::pos[9].x += move_dist;
		}
		if (cl_float3_dist(Kernel::pos[9], Kernel::pos[14]) < max_interval) {
			Kernel::pos[14].x += move_dist;
		}
		if (cl_float3_dist(Kernel::pos[14], Kernel::pos[19]) < max_interval) {
			Kernel::pos[19].x += move_dist;
		}
		std::cout << "pins pos: " << Kernel::pos[0].x << " " << Kernel::pos[4].x << " ";
		std::cout << Kernel::pos[9].x << " " << Kernel::pos[14].x << " " << Kernel::pos[19].x << std::endl;
	}

}

float cl_float3_dist(cl_float3& v1, cl_float3& v2) {
	float x = v1.x - v2.x;
	float y = v1.y - v2.y;
	float z = v1.z - v2.z;

	return sqrtf(x * x + y * y + z * z);
}

static void
framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	Globals::win_width = float(width);
	Globals::win_height = float(height);
	Globals::aspect = Globals::win_width / Globals::win_height;

	glViewport(0, 0, width, height);

	// ToDo: update the perspective matrix as the window size changes
	Globals::frus.set_frustum(60.0, Globals::aspect, 0.1f, 100.f);
	set_projection_mat();
}
