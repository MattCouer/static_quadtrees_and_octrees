#ifndef Z_OCTREE_H
#define Z_OCTREE_H

#include "cell_neighbors.h"
#include "z_locational_code.h"
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

#define ZOCTREE_UPPERBOUND 19

class ZOcTree : public RefCounted {
	GDCLASS(ZOcTree, RefCounted)

public:

private:


	

protected:
	static void _bind_methods();

public:
	
	ZOcTree();
	~ZOcTree();
    
    int64_t total_depth;
    real_t initial_octant_span;
    Vector3 initial_octant_center;

	static Ref<ZOcTree> create(int64_t _total_depth, Vector3 _initial_quadrant_center, real_t _initial_quadrant_span);


    int64_t cut_and_dilate(int64_t x);
    int64_t mask_and_contract(int64_t x);

    int64_t morton_encode_xyz(int64_t x, int64_t y, int64_t z);
    int64_t morton_decode_x(int64_t code);
    int64_t morton_decode_y(int64_t code);
    int64_t morton_decode_z(int64_t code);

    int64_t floating_to_integral_x(real_t x, int64_t at_depth);
    int64_t floating_to_integral_y(real_t y, int64_t at_depth);
    int64_t floating_to_integral_z(real_t z, int64_t at_depth);

    int64_t get_bit_upper_bound() {return ZOCTREE_UPPERBOUND; }

	int64_t find_depth();
	TypedArray<ZLocationalCode> layout_leaves();
	Ref<ZLocationalCode> leaf_that_contains_point(Vector3 point);
    Ref<ZLocationalCode> cell_that_contains_point(Vector3 point, int64_t at_depth);
    // TypedArray<ZLocationalCode> simple_octants_that_intersect_ray(int64_t depth, Vector3 ray_origin, Vector3 ray_direction);

    Vector3 get_cell_center(Ref<ZLocationalCode> octant);
    real_t get_cell_span(int64_t depth);
    // bool octant_slab_test(Ref<ZLocationalCode> octant, Vector3 origin, Vector3 direction);
	Ref<ZLocationalCode> get_cell_parent(Ref<ZLocationalCode> octant);
    TypedArray<ZLocationalCode> get_cell_children(Ref<ZLocationalCode> octant);

	Ref<ZLocationalCode> find_neighbor_of_equal_depth(Ref<ZLocationalCode> octant, int64_t neighbor);
	TypedArray<ZLocationalCode> find_neighbors_of_equal_depth(Ref<ZLocationalCode> octant, PackedInt64Array neighbors);
	// TypedArray<ZLocationalCode> find_leaf_neighbors(Ref<ZLocationalCode> octant, int64_t neighbor);

    int64_t morton_add_x_and_null_out_of_domain(int64_t x, int64_t depth, int64_t term);
    int64_t morton_add_y_and_null_out_of_domain(int64_t y, int64_t depth, int64_t term);
    int64_t morton_add_z_and_null_out_of_domain(int64_t z, int64_t depth, int64_t term);

    void split(int64_t _total_depth);
    void readjust(Vector3 _initial_octant_center, real_t _initial_octant_span);

	Array gen_arrays(int64_t at_depth);
    Array gen_z_arrays(int64_t at_depth);
};

}

#endif