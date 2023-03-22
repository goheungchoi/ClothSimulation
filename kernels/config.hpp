#ifndef _CONFIG_HPP
#define _CONFIG_HPP

// cloth info
#define _PINNED
#define CLOTH_TOP 10.f

#ifdef _PINNED
	#define CLOTH_WIDTH 19.f
	#define CLOTH_HEIGHT 19.f
	#define CLOTH_ROW 19
	#define CLOTH_COL 19
#else
	#define CLOTH_WIDTH 40.f
	#define CLOTH_HEIGHT 40.f
	#define CLOTH_ROW 40
	#define CLOTH_COL 40
#endif

#define DELTA_TIME (1.0f / 60.0f)
#define GRAVITY 5.f
#define TAU 0.010f	// stiffness - Carpet
//#define TAU 0.115f	// stiffness - Carpet
//#define TAU 0.015f	// stiffness - Tablecloth
//#define TAU 0.003f	// stiffness -  Swimming suit
#define SOLVER_ITERATIONS 9
#define KD 0.02f	// damping constant - Carpet
//#define KD 0.02f		// damping constant - Tablecloth
//#define KD 0.015f	// damping constant - Shirt

#define BLOCK_SIZE 1

#define SPHERE_SCALE 5.0f

#endif