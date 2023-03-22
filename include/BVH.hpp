//#ifndef _BVH_HPP_
//#define _BVH_HPP_
//
//#include <vector>
//#include "vector.hpp"
//
//// k = 18
//typedef struct k_DOP {
//	static std::vector<Vec3f> directions{
//		Vec3f(1, 0, 0),
//		Vec3f(0, 1, 0),
//		Vec3f(0, 0, 1),
//		Vec3f(1, 1, 0),
//		Vec3f(1, 0, 1),
//		Vec3f(0, 1, 1),
//		Vec3f(1,-1, 0),
//		Vec3f(1, 0,-1),
//		Vec3f(0, 1,-1) };
//
//	Vec3f center;
//	std::vector<float> min_values;
//	std::vector<float> max_values;
//
//} k_DOP;
//
//k_DOP createkDOP(std::vector<Vec3f> vertices)
//{
//	k_DOP result;
//	result.center = Vec3f(0.f, 0.f, 0.f);
//
//	for (int i = 0; i < vertices.size(); i++) {
//		result.center += vertices[i];
//	}
//	result.center[0] /= vertices.size(); 
//	result.center[1] /= vertices.size(); 
//	result.center[2] /= vertices.size();
//
//	for (int j = 0; j < result.directions.size(); j++) {
//		result.min_values.push_back(FLT_MAX);
//		result.max_values.push_back(FLT_MIN);
//	}
//
//	for (int i = 0; i < vertices.size(); i++) {
//		Vec3f X = vertices[i];
//		for (int j = 0; j < result.directions.size(); j++) {
//			Vec3f v = result.directions[j]; 
//			Vec3f O = result.center;
//			float t = X.dot(v) - O.dot(v);
//			t = t / v.dot(v);
//			if (t < result.min_values[j]) result.min_values[j] = t;
//			if (t > result.max_values[j]) result.max_values[j] = t;
//		}
//	} 
//
//	return result;
//}
//
//
//typedef struct bvh {
//	TreeNode* root = nullptr;
//
//
//} BVH;
//
//typedef struct tree_node {
//	K_DOP bound;
//	TreeNode* left = nullptr;
//	TreeNode* right = nullptr;
//} TreeNode;
//
//
//#endif // !_BVH_HPP_
