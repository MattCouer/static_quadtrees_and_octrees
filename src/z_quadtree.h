#ifndef Z_QUADTREE_H
#define Z_QUADTREE_H

#include "cell_neighbors.h"
#include "z_locational_code.h"
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

#define ZQUADTREE_UPPERBOUND 27

class ZQuadTree : public RefCounted {
	GDCLASS(ZQuadTree, RefCounted)

public:

private:



	

protected:
	static void _bind_methods();

public:
	

	ZQuadTree();
	~ZQuadTree();
    
    int64_t total_depth;
    real_t initial_quadrant_span;
    Vector2 initial_quadrant_center;

	static Ref<ZQuadTree> create(int64_t _total_depth, Vector2 _initial_quadrant_center, real_t _initial_quadrant_span);

    int64_t cut_and_dilate(int64_t x);
    int64_t mask_and_contract(int64_t x);

    int64_t morton_encode_xy(int64_t x, int64_t y);
    int64_t morton_decode_x(int64_t code);
    int64_t morton_decode_y(int64_t code);

    int64_t floating_to_integral_x(real_t x, int64_t at_depth);
    int64_t floating_to_integral_y(real_t y, int64_t at_depth);

    int64_t get_bit_upper_bound() {return ZQUADTREE_UPPERBOUND; }

	int64_t find_depth();
	TypedArray<ZLocationalCode> layout_leaves();
    TypedArray<ZLocationalCode> layout_level(int64_t at_depth);
	Ref<ZLocationalCode> leaf_that_contains_point(Vector2 point);
    Ref<ZLocationalCode> cell_that_contains_point(Vector2 point, int64_t at_depth);
    // TypedArray<ZLocationalCode> simple_quadrants_that_intersect_ray(int64_t depth, Vector2 ray_origin, Vector2 ray_direction);
    
    Vector2 get_cell_center(Ref<ZLocationalCode> quadrant);
    real_t get_cell_span(int64_t depth);
    // bool quadrant_slab_test(Ref<ZLocationalCode> quadrant, Vector2 origin, Vector2 direction);
    Ref<ZLocationalCode> get_cell_parent(Ref<ZLocationalCode> quadrant);
    TypedArray<ZLocationalCode> get_cell_children(Ref<ZLocationalCode> quadrant);

	Ref<ZLocationalCode> find_neighbor_of_equal_depth(Ref<ZLocationalCode> quadrant, int64_t neighbor);
	TypedArray<ZLocationalCode> find_neighbors_of_equal_depth(Ref<ZLocationalCode> quadrant, PackedInt64Array neighbors);
	// TypedArray<ZLocationalCode> find_leaf_neighbors(Ref<ZLocationalCode> quadrant, int64_t neighbor);
    
    int64_t morton_add_and_null_out_of_domain(int64_t code, int64_t x_term, int64_t y_term, int64_t depth);

    void split(int64_t _total_depth);
    void readjust(Vector2 _initial_quadrant_center, real_t _initial_quadrant_span);

	PackedVector2Array gen_lines(int64_t at_depth);
	PackedVector2Array gen_z_lines(int64_t at_depth);
};

}



#endif