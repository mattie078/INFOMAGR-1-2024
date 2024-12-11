#ifndef GLOBALS_H
#define GLOBALS_H

#include <atomic>

#define DEBUG
#ifdef DEBUG 
#define D(x) (x)
#else 
#define D(x) do{}while(0)
#endif

extern std::atomic<uint32_t> num_ray_triangle_tests; 
extern std::atomic<uint32_t> num_ray_triangle_intersections; 
extern std::atomic<uint32_t> num_primary_rays;
extern std::atomic<uint32_t> num_triangles;

extern std::atomic<uint32_t> num_ray_kdtree_node_tests;
extern std::atomic<uint32_t> num_ray_kdtree_node_intersections;
extern std::atomic<uint32_t> num_kdtree_nodes; 
extern std::atomic<uint32_t> num_kdtree_leaf_nodes; 
extern std::atomic<uint32_t> num_ray_kdtree_leaf_tests;
extern std::atomic<uint32_t> num_ray_kdtree_leaf_intersections;

#endif // GLOBALS_H
