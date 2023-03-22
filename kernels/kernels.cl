#include "config.hpp"

#define index(i, j) j+(CLOTH_COL+1)*i
#define offset(v_offset, h_offset) new_position[(CLOTH_COL+1)*(i+v_offset) + j+h_offset] 


__kernel void update_position(__global float3* old_positions,
                              __global float3* positions,
                              __global float3* new_position)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    size_t idx = index(i, j);
    if (i < 0 || j < 0 || i > CLOTH_ROW || j > CLOTH_COL)
        return;
    
#ifdef _PINNED
    if (idx == 0 || idx == 4 || idx == 9 || idx == 14 || idx == 19)
        new_position[idx] = positions[idx];
#endif

    float dt = DELTA_TIME;
    float3 gravity = {0.0f, -GRAVITY, 0.f};
    float kd = KD;

    float3 vel = (1.f - kd) * (positions[idx] - old_positions[idx]);
    float3 acc = gravity*dt*dt;
    
    new_position[idx] = positions[idx] + vel + acc;
}

__kernel void update_old_position(__global float3* old_positions,
                                  __global float3* positions)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    size_t idx = index(i, j);
    
    if (i < 0 || j < 0 || i > CLOTH_ROW || j > CLOTH_COL)
        return;
    
    old_positions[idx] = positions[idx];
}

float3 dynamic_inverse(float3 first, float3 second, float restDist)
{
    float tau = TAU;

    float3 v = second - first;
    float dist = fast_length((float4)(v, 1.f));  // distance from first to second
    float deformationRate = (dist - restDist) / dist;
    return tau*deformationRate*v;
}

__kernel void constraint(__global float3* new_position,
                         __global float3* positions)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    size_t idx = index(i, j);

#ifdef _PINNED
    if (i < 0 || j < 0 || i > CLOTH_ROW || j > CLOTH_COL ||
        idx == 0 || idx == 4 || idx == 9 || idx == 14 || idx == 19)
        return;
#else
    if (i < 0 || j < 0 || i > CLOTH_ROW || j > CLOTH_COL)
        return;
#endif
    
    float3 output = new_position[idx];
    //printf("new_position = %2.2v3hlf\n", output);

    float3 delta = {0.0f, 0.0f, 0.0f};
    
    
	const float dx = CLOTH_WIDTH / CLOTH_COL;
    const float dy = CLOTH_HEIGHT / CLOTH_ROW;

	if (i > 0)
        delta += dynamic_inverse(output, offset(-1,  0), dy);
	if (i < (CLOTH_ROW))
		delta += dynamic_inverse(output, offset(+1,  0), dy);
	if (j < (CLOTH_COL))
		delta += dynamic_inverse(output, offset( 0, +1), dx);
	if (j > 0)
		delta += dynamic_inverse(output, offset( 0, -1), dx);
    
	const float diagl = sqrt(dy*dy + dx*dx);
    
	if (i > 0 && j > 0)
		delta += dynamic_inverse(output, offset(-1, -1), diagl);
	if (i < (CLOTH_ROW) && j > 0)
		delta += dynamic_inverse(output, offset(+1, -1), diagl);
	if (i > 0 && j < (CLOTH_COL))
		delta += dynamic_inverse(output, offset(-1, +1), diagl);
	if (i < (CLOTH_ROW) && j < (CLOTH_COL))
		delta += dynamic_inverse(output, offset(+1, +1), diagl);
	
	const float dblDiagl = 2.0f * diagl;
    
	if (i > 1 && j > 1)
		delta += dynamic_inverse(output, offset(-2, -2), dblDiagl);
	if (i < (CLOTH_ROW-1) && j > 1)
		delta += dynamic_inverse(output, offset(+2, -2), dblDiagl);
	if (i > 1 && j < (CLOTH_COL-1))
		delta += dynamic_inverse(output, offset(-2, +2), dblDiagl);
	if (i < (CLOTH_ROW-1) && j < (CLOTH_COL-1))
		delta += dynamic_inverse(output, offset(+2, +2), dblDiagl);

	output += delta;

    // COLLISION DETECTION
//#ifndef _PINNED
    float r = 5.5f;
    float3 o = {0.0f, 0.0f, 0.0f};
    
    float3 v = o - output;
    float dist = fast_length((float4)(v, 1.f));
    if (dist < r)
    {
        float diff = (dist - r) / dist;
        output += v * diff;
    }
//#endif
    positions[idx] = output;
}

float3 clamp_pos(__global float3* positions, int i, int j)
{
    i = max(0, min(CLOTH_ROW, i));
    j = max(0, min(CLOTH_COL, j));
    size_t idx = index(i, j);
    return positions[idx];
}

__kernel void calculate_normals(__global float3* positions,
                                __global float3* normals)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    size_t idx = index(i, j);
    
    if (i < 0 || j < 0 || i > CLOTH_ROW || j > CLOTH_COL)
        return;

	float3 output = positions[idx];
	float3 down    = clamp_pos(positions, i + 1, j);
	float3 up  = clamp_pos(positions, i - 1, j);
	float3 right = clamp_pos(positions, i, j + 1);
	float3 left  = clamp_pos(positions, i, j - 1);
    
	float3 sum = {0.0f, 0.0f, 0.0f};

    sum += cross(left - output, up - output);
    sum += cross(down - output, left - output);
    sum += cross(right - output, down - output);
    sum += cross(up - output, right - output);
    
    normals[idx] = sum / fast_length(sum);
}