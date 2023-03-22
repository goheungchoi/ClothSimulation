//#ifndef _COLLISION_HPP_
//#define _COLLISION_HPP_
//
//#include <vector>
//#include <cmath>
//#include "vector.hpp"
//#include "trimesh.hpp"
//
//typedef struct normal_cone {
//	float alpha;
//	Vec3f l;
//	std::vector<Vec3f> contour;
//
//	normal_cone(float alpha, Vec3f l, std::vector<Vec3f> contour)
//	: alpha(alpha), l(l), contour(contour) {}
//
//} NormalCone;
//
//NormalCone calculateNormalCone(TriMesh &mesh) {
//	float alpha = 0.f;
//	Vec3f l = mesh.normals[0];
//	
//	for (int i = 1; i < mesh.normals.size(); i++) {
//		float beta = Math.acos(l.dot(mesh.normals[i]));
//		if (alpha < beta) {
//
//		}
//
//	}
//
//}
//
//
//#endif