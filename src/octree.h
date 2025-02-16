#ifndef OCTREE_H
#define OCTREE_H

#include "flat_octree_octant.h"
#include "octree_octant.h"
#include "cell_neighbors.h"
#include <godot_cpp/classes/ref_counted.hpp>
// #include <godot_cpp/classes/thread.hpp>
//#include <godot_cpp/classes/os.hpp>


namespace godot {

class OcTree : public RefCounted {
	GDCLASS(OcTree, RefCounted)

public:

private:

public:
		// THIS IS THE ORDER OF THE CHILDREN
	enum OCTANT_POSITION {	
	BOTTOM_SOUTH_WEST = 0,
	BOTTOM_NORTH_WEST = 1,
	BOTTOM_SOUTH_EAST = 2,
	BOTTOM_NORTH_EAST = 3,
	TOP_SOUTH_WEST = 4,
	TOP_NORTH_WEST = 5,
	TOP_SOUTH_EAST = 6,
	TOP_NORTH_EAST = 7,
	};

	OcTree();
	~OcTree();

    Ref<OcTreeOctant> root;
	int64_t maximum_depth;
	// TypedArray<Thread> threads;
	// bool single_threaded;

	TypedArray<FlatOcTreeOctant> flatten();
	TypedArray<OcTreeOctant> linearize();
	void linearize_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> &arr);
	static TypedArray<OcTreeOctant> flat_to_linear(TypedArray<FlatOcTreeOctant> flat_array);
	static void flat_to_linear_recursive(TypedArray<FlatOcTreeOctant> flat_array, TypedArray<OcTreeOctant> &linear_array, int64_t idx, int64_t child_idx);

	GDVIRTUAL0R(Dictionary, _generate_meta_dictionary);
	Dictionary generate_meta_dictionary();

	static TypedArray<Dictionary> json_serialize(Ref<OcTree> octree);
	static Ref<godot::OcTree> json_deserialize(TypedArray<Dictionary> data);

	Ref<OcTreeOctant> get_root();
	void set_root(Ref<OcTreeOctant> _root);
	int64_t get_maximum_depth();
	void set_maximum_depth(int64_t p_maximum_depth);
	int64_t cell_count();
	void cell_count_recursive(Ref<OcTreeOctant> root, int64_t &ct);

    void split_on_cell(Callable condition_on_cell);
	void split_on_condition(Callable condition);
	void split_recursive_on_cell(Ref<OcTreeOctant> root, Callable condition);
	void split_recursive_on_condition(Ref<OcTreeOctant> root, Callable condition);
	int64_t find_depth();
	TypedArray<OcTreeOctant> layout_leaves();
	void layout_leaves_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> leaves);
	Ref<OcTreeOctant>  leaf_that_contains_point(Vector3 point);
	Ref<OcTreeOctant> leaf_that_contains_point_recursive(Ref<OcTreeOctant> root, Vector3 point);
	Ref<OcTreeOctant> cell_that_contains_point(Vector3 point, int64_t at_depth);
	Ref<OcTreeOctant> cell_that_contains_point_recursive(Ref<OcTreeOctant> root, Vector3 point, int64_t at_depth);
		
	TypedArray<OcTreeOctant> cells_that_intersect_ray(Vector3 ray_origin, Vector3 ray_direction);
    void cells_that_intersect_ray_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> &intersected, Vector3 ray_origin, Vector3 ray_direction);
	

	Ref<OcTreeOctant> find_neighbor_for_traversal(Ref<OcTreeOctant> octant, int64_t neighbor);
	Ref<OcTreeOctant> find_neighbor_of_equal_depth(Ref<OcTreeOctant> octant, int64_t neighbor);
	TypedArray<OcTreeOctant> find_neighbors_of_equal_depth(Ref<OcTreeOctant> octant, PackedInt64Array neighbors);
	TypedArray<OcTreeOctant> find_leaf_neighbors(Ref<OcTreeOctant> octant, int64_t neighbor);

	Array gen_arrays();

	void gen_arrays_recursive(Ref<OcTreeOctant> root, PackedVector3Array &vertices, PackedInt32Array &indices);

protected:
	static void _bind_methods();

};

//Used by OcTree::find_neighbor_of_equal_depth()
//DIM 0: OCTANT_POSITION
//DIM 1: OCTANT_NEIGHBOR
//DIM 2: Path to neighbor [0] is -1 if neighbor is a direct sibling
static const int64_t octree_eqd_neighbor_table [8][26][2] = {
													{
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_WEST,OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_WEST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_WEST, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_WEST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														}
													},
													{
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														}
													},
													{
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															-1, OcTree::OCTANT_POSITION::TOP_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_EAST, OcTree::OCTANT_POSITION::TOP_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_NORTH_EAST, OcTree::OCTANT_POSITION::TOP_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_NORTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_EAST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST, OcTree::OCTANT_POSITION::BOTTOM_NORTH_WEST
														},
														{
															OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_EAST, OcTree::OCTANT_POSITION::BOTTOM_SOUTH_WEST
														}
													}
												};

}







#endif