#ifndef QUADTREE_H
#define QUADTREE_H

#include "flat_quadtree_quadrant.h"
#include "quadtree_quadrant.h"
#include "cell_neighbors.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
// #include <godot_cpp/classes/thread.hpp>
//#include <godot_cpp/classes/os.hpp>

namespace godot {

class QuadTree : public RefCounted {
	GDCLASS(QuadTree, RefCounted)


public:


private:


protected:
	static void _bind_methods();

public:
	
	// THIS IS THE ORDER OF THE CHILDREN
	enum QUADRANT_POSITION {
	SOUTH_WEST = 0,
	NORTH_WEST = 1,
	SOUTH_EAST = 2,
	NORTH_EAST = 3,
	};
	
	QuadTree();
	~QuadTree();

	Ref<QuadTreeQuadrant> root;
	int64_t maximum_depth;
    // TypedArray<Thread> threads;
    // bool single_threaded;


	TypedArray<FlatQuadTreeQuadrant> flatten();
    TypedArray<QuadTreeQuadrant> linearize();
    void linearize_recursive(Ref<QuadTreeQuadrant> _root, TypedArray<QuadTreeQuadrant> &arr);
    static TypedArray<QuadTreeQuadrant> flat_to_linear(TypedArray<FlatQuadTreeQuadrant> flat_array);
    static void flat_to_linear_recursive(TypedArray<FlatQuadTreeQuadrant> flat_array, TypedArray<QuadTreeQuadrant> &linear_array, int64_t idx, int64_t child_idx);

    GDVIRTUAL0R(Dictionary, _generate_meta_dictionary);
	Dictionary generate_meta_dictionary();
    
    static TypedArray<Dictionary> json_serialize(Ref<QuadTree> quadtree);
    static Ref<godot::QuadTree> json_deserialize(TypedArray<Dictionary> data);

	Ref<QuadTreeQuadrant> get_root();
    void set_root(Ref<QuadTreeQuadrant> _root);
	int64_t get_maximum_depth();
	void set_maximum_depth(int64_t p_maximum_depth);
    int64_t cell_count();
	void cell_count_recursive(Ref<QuadTreeQuadrant> root, int64_t ct);

   	void split_on_cell(Callable condition_on_cell);
	void split_on_condition(Callable condition);
	void split_recursive_on_cell(Ref<QuadTreeQuadrant> root, Callable condition);
	void split_recursive_on_condition(Ref<QuadTreeQuadrant> root, Callable condition);

	int64_t find_depth();
	TypedArray<QuadTreeQuadrant> layout_leaves();
	void layout_leaves_recursive(Ref<QuadTreeQuadrant> root, TypedArray<QuadTreeQuadrant>& leaves);
	Ref<QuadTreeQuadrant> leaf_that_contains_point(Vector2 point);
	Ref<QuadTreeQuadrant> leaf_that_contains_point_recursive(Ref<QuadTreeQuadrant> root, Vector2 point);
	TypedArray<QuadTreeQuadrant> quadrants_that_intersect_ray(Vector2 ray_origin, Vector2 ray_direction);
    void quadrants_that_intersect_ray_recursive(Ref<QuadTreeQuadrant> root, TypedArray<QuadTreeQuadrant> &intersected, Vector2 ray_origin, Vector2 ray_direction);	

	Ref<QuadTreeQuadrant> find_neighbor_for_traversal(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor);
	Ref<QuadTreeQuadrant> find_neighbor_of_equal_depth(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor);
	TypedArray<QuadTreeQuadrant> find_neighbors_of_equal_depth(Ref<QuadTreeQuadrant> quadrant, PackedInt64Array neighbors);
	TypedArray<QuadTreeQuadrant> find_leaf_neighbors(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor);

	PackedVector2Array gen_lines();

	void gen_lines_recursive(Ref<QuadTreeQuadrant> root, PackedVector2Array &array);
};


//Used by QuadTree::find_neighbor_of_equal_depth()
//DIM 0: QUADRANT_POSITION
//DIM 1: QUADRANT_NEIGHBOR
//DIM 2: Path to neighbor, [0] is -1 if neighbor is not a direct sibling
static const int64_t quadtree_eqd_neighbor_table [4][8][2] = {
                                                    {
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_EAST
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_WEST,-1
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_EAST
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_EAST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_EAST,-1
                                                        },
                                                    },
                                                    {
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_WEST
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_WEST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_EAST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_EAST,-1
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_EAST
                                                        },
                                                    },
                                                    {
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_WEST
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_WEST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_WEST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_EAST,-1
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_EAST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_WEST
                                                        },
                                                    },
                                                    {
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_WEST,-1
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::NORTH_WEST-1
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_EAST
                                                        },
                                                        {
                                                            QuadTree::QUADRANT_POSITION::SOUTH_EAST,-1
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::NORTH_WEST
                                                        },
                                                        {
                                                            -1,QuadTree::QUADRANT_POSITION::SOUTH_WEST
                                                        },
                                                    },
                                                };

}




#endif