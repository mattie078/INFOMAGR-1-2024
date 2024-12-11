#include "globals.h"

std::atomic<uint32_t> num_ray_triangle_tests(0); 
std::atomic<uint32_t> num_ray_triangle_intersections(0); 
std::atomic<uint32_t> num_primary_rays(0);
std::atomic<uint32_t> num_triangles(0);

std::atomic<uint32_t> num_ray_kdtree_node_tests(0);
std::atomic<uint32_t> num_ray_kdtree_node_intersections(0);
std::atomic<uint32_t> num_kdtree_nodes(0); 
std::atomic<uint32_t> num_kdtree_leaf_nodes(0); 
std::atomic<uint32_t> num_ray_kdtree_leaf_tests(0);
std::atomic<uint32_t> num_ray_kdtree_leaf_intersections(0);
