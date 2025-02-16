#include "octree.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;


void OcTree::_bind_methods(){
    ClassDB::bind_method(D_METHOD("flatten"), &OcTree::flatten);
    ClassDB::bind_static_method("OcTree", D_METHOD("json_serialize", "octree"), &OcTree::json_serialize);
    ClassDB::bind_static_method("OcTree", D_METHOD("json_deserialize", "dictionary_array"), &OcTree::json_deserialize);
    ClassDB::bind_method(D_METHOD("cell_count"), &OcTree::cell_count);
    ClassDB::bind_method(D_METHOD("split_on_cell", "condition_on_cell"), &OcTree::split_on_cell);
	ClassDB::bind_method(D_METHOD("split_on_condition", "condition"), &OcTree::split_on_condition);
    ClassDB::bind_method(D_METHOD("find_depth"), &OcTree::find_depth);
    ClassDB::bind_method(D_METHOD("layout_leaves"), &OcTree::layout_leaves);
	ClassDB::bind_method(D_METHOD("leaf_that_contains_point", "point"), &OcTree::leaf_that_contains_point);
	ClassDB::bind_method(D_METHOD("cell_that_contains_point", "point", "at_depth"), &OcTree::cell_that_contains_point);
	ClassDB::bind_method(D_METHOD("cells_that_intersect_ray", "ray_origin", "ray_direction"), &OcTree::cells_that_intersect_ray);
    ClassDB::bind_method(D_METHOD("find_neighbor_for_traversal", "octant", "neighbor"), &OcTree::find_neighbor_for_traversal);
    ClassDB::bind_method(D_METHOD("find_neighbor_of_equal_depth", "octant", "neighbor"), &OcTree::find_neighbor_of_equal_depth);
	ClassDB::bind_method(D_METHOD("find_neighbors_of_equal_depth", "octant", "neighbors"), &OcTree::find_neighbors_of_equal_depth);
    ClassDB::bind_method(D_METHOD("find_leaf_neighbors", "octant", "neighbor"), &OcTree::find_leaf_neighbors);
	ClassDB::bind_method(D_METHOD("gen_arrays"), &OcTree::gen_arrays);

    ClassDB::bind_method(D_METHOD("set_root", "root"), &OcTree::set_root);
    ClassDB::bind_method(D_METHOD("get_root"), &OcTree::get_root);
	ClassDB::bind_method(D_METHOD("set_maximum_depth", "maximum_depth"), &OcTree::set_maximum_depth);
    ClassDB::bind_method(D_METHOD("get_maximum_depth"), &OcTree::get_maximum_depth);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "maximum_depth"), "set_maximum_depth", "get_maximum_depth");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root"), "set_root", "get_root");

    BIND_ENUM_CONSTANT(OCTANT_BELOW_SOUTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_WEST);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_NORTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_NORTH);
    BIND_ENUM_CONSTANT(OCTANT_BELOW);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_SOUTH);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_SOUTH_EAST);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_EAST);
    BIND_ENUM_CONSTANT(OCTANT_BELOW_NORTH_EAST);
    BIND_ENUM_CONSTANT(OCTANT_SOUTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_WEST);
    BIND_ENUM_CONSTANT(OCTANT_NORTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_NORTH);
    BIND_ENUM_CONSTANT(OCTANT_SOUTH);
    BIND_ENUM_CONSTANT(OCTANT_SOUTH_EAST);
    BIND_ENUM_CONSTANT(OCTANT_EAST);
    BIND_ENUM_CONSTANT(OCTANT_NORTH_EAST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_SOUTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_WEST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_NORTH_WEST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_NORTH);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_SOUTH);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_SOUTH_EAST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_EAST);
    BIND_ENUM_CONSTANT(OCTANT_ABOVE_NORTH_EAST);

    //VIRTUAL METHODS
    GDVIRTUAL_BIND(_generate_meta_dictionary);

}

OcTree::OcTree(){
	
}
OcTree::~OcTree(){
    
}

TypedArray<FlatOcTreeOctant> OcTree::flatten(){
    TypedArray<FlatOcTreeOctant> res;
    TypedArray<OcTreeOctant> linear = linearize();
    res.resize(linear.size());
    Ref<FlatOcTreeOctant> input;
    Ref<OcTreeOctant> match;
    for(int64_t i = 0; i < linear.size(); i++){
        match = linear[i];
        input = match->flatten();
        if(!match->is_root()){
            input->parent = -1;
            for(int64_t j = 0; j < linear.size(); j++){
                Ref<OcTreeOctant> query = linear[j];
                if(query.ptr() == match->parent){
                    input->parent = j;
                    break;
                }
            }
        }
        if(!match->is_leaf())
            for(int64_t j = 0; j < 8; j++)
                input->children[j] = linear.find(match->children[j]);
        
        res[i] = input;
    }
    return res;
}


TypedArray<OcTreeOctant> OcTree::linearize(){
    TypedArray<OcTreeOctant> res;
    linearize_recursive(get_root(), res);
    return res;
}
void OcTree::linearize_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> &arr){
    arr.push_back(root);
    for(int64_t i = 0; i < root->children.size(); i++)
        if(Ref<OcTreeOctant>(root->children[i]).is_valid())
            linearize_recursive(root->children[i], arr);
}

TypedArray<OcTreeOctant> OcTree::flat_to_linear(TypedArray<FlatOcTreeOctant> flat_array){
    TypedArray<OcTreeOctant> res;
    flat_to_linear_recursive(flat_array, res, 0, 0);
    return res;
}


void OcTree::flat_to_linear_recursive(TypedArray<FlatOcTreeOctant> flat_array, TypedArray<OcTreeOctant> &linear_array, int64_t idx, int64_t child_idx){

    Ref<FlatOcTreeOctant> flat_octant = flat_array[idx];
    Ref<OcTreeOctant> new_octant = flat_octant->unflatten();

    //If octant is not root
    if(flat_octant->parent != -1){
        //Associate parent
        new_octant->parent = Object::cast_to<OcTreeOctant>(linear_array[flat_octant->parent]);
        Ref<OcTreeOctant> parent_ref = Ref<OcTreeOctant>(new_octant->parent);
        // Ref<OcTreeOctant> parent_ref = Ref<OcTreeOctant>(Object::cast_to<OcTreeOctant>(new_octant->parent));
        //If parent hasn't been touched, resize
        if(parent_ref->children.size() != 8)
            parent_ref->children.resize(8);
        //Add as child
        parent_ref->children[child_idx] = new_octant;
    }

    linear_array.push_back(new_octant);

    for(int64_t i = 0; i < flat_octant->children.size(); i++)
        if(flat_octant->children[i] != -1)
            flat_to_linear_recursive(flat_array, linear_array, flat_octant->children[i], i);

}

Dictionary OcTree::generate_meta_dictionary(){
    Dictionary meta;
    if(GDVIRTUAL_IS_OVERRIDDEN(_generate_meta_dictionary)){
        _gdvirtual__generate_meta_dictionary_call<false>(meta);
    }else{
        meta.get_or_add("maximum_depth", maximum_depth);
    }
    return meta;
}

TypedArray<Dictionary> OcTree::json_serialize(Ref<OcTree> octree){
    TypedArray<Dictionary> res;
    TypedArray<FlatOcTreeOctant> flat_array = octree->flatten();

    Dictionary meta_dictionary = octree->generate_meta_dictionary();
    res.resize(flat_array.size() + 1);
    res[0] = meta_dictionary;
    

    Ref<FlatOcTreeOctant> flat_octant;
    for(int64_t i = 0; i < flat_array.size(); i++){
        flat_octant = flat_array[i];
        res[i+1] = flat_octant->to_dict();
    }
    return res;
}
Ref<godot::OcTree> OcTree::json_deserialize(TypedArray<Dictionary> data){
    if(!data[0].has_key("maximum_depth")){
        //print err
        return Ref<godot::OcTree>();
    }

    
    TypedArray<FlatOcTreeOctant> flat_array;
    flat_array.resize(data.size()-1);//-1 in assumption of meta
    Ref<FlatOcTreeOctant> flat_octant;
    Dictionary current_dictionary;
    for(int64_t i = 1; i < data.size(); i++){
        current_dictionary = data[i];
        flat_octant.instantiate();
        flat_octant->copy_from_dict(current_dictionary);
        flat_array[i-1] = flat_octant;
    }
    
    TypedArray<OcTreeOctant> linear = flat_to_linear(flat_array);

    Dictionary meta = data[0];
    Ref<godot::OcTree> res;
    res.instantiate();
    res->root = linear[0];
    res->maximum_depth = meta["maximum_depth"];
    for(int64_t i = 0; i < meta.keys().size(); i++){
        Variant key = meta.keys()[i];
        if(key == "maximum_depth")
            continue;
        res->set(key, meta[key]);
    }

    return res;
}

Ref<OcTreeOctant> OcTree::get_root(){
    return root;
}
void OcTree::set_root(Ref<OcTreeOctant> _root){
    root = _root;
}
int64_t OcTree::get_maximum_depth(){
    return maximum_depth;
}
void OcTree::set_maximum_depth(int64_t p_maximum_depth){
    maximum_depth = p_maximum_depth;
}

int64_t OcTree::cell_count(){
    int64_t ct = 0;
    cell_count_recursive(root, ct);
    return ct;
}
void OcTree::cell_count_recursive(Ref<OcTreeOctant> root, int64_t &ct){
    if(root.is_null())
        return;
    ct++;
    for(int64_t i = 0; i < root->children.size(); i++)
        cell_count_recursive(root->children[i], ct);
}

void OcTree::split_on_cell(Callable condition_on_cell){
    split_recursive_on_cell(root, condition_on_cell);   
}


void OcTree::split_on_condition(Callable condition){
    split_recursive_on_condition(root, condition);
}

void OcTree::split_recursive_on_cell(Ref<OcTreeOctant> root, Callable condition){
    Ref<OcTreeOctant> current = root;
    uint64_t depth = 0;
    while(!(current->is_root())){
        depth++;
        current = Ref<OcTreeOctant>(current->parent);
    }

    if (depth >= maximum_depth)
        return;

    Variant result = condition.call(root);

    if(result.operator bool()) {
        root->children.resize(8);
        Vector3 bsw_center = root->center - Vector3(root->span/2, root->span/2, root->span/2);
        Ref<OcTreeOctant> new_octant_bsw;
        new_octant_bsw = root->prototype();
        new_octant_bsw->center = bsw_center;
        new_octant_bsw->span = root->span/2;
        new_octant_bsw->parent = root.ptr();
        root->children[0] = new_octant_bsw;
        Ref<OcTreeOctant> new_octant_bnw;
        new_octant_bnw = root->prototype();
        new_octant_bnw->center = bsw_center + Vector3(0, 0, root->span);
        new_octant_bnw->span = root->span/2;
        new_octant_bnw->parent = root.ptr();
        root->children[1] = new_octant_bnw;
        Ref<OcTreeOctant> new_octant_bse;
        new_octant_bse = root->prototype();
        new_octant_bse->center = bsw_center + Vector3(root->span, 0, 0);
        new_octant_bse->span = root->span/2;
        new_octant_bse->parent = root.ptr();
        root->children[2] = new_octant_bse;
        Ref<OcTreeOctant> new_octant_bne;
        new_octant_bne = root->prototype();
        new_octant_bne->center = bsw_center + Vector3(root->span, 0, root->span);
        new_octant_bne->span = root->span/2;
        new_octant_bne->parent = root.ptr();
        root->children[3] = new_octant_bne;
        Ref<OcTreeOctant> new_octant_tsw;
        new_octant_tsw = root->prototype();
        new_octant_tsw->center = bsw_center + Vector3(0, root->span, 0);
        new_octant_tsw->span = root->span/2;
        new_octant_tsw->parent = root.ptr();
        root->children[4] = new_octant_tsw;
        Ref<OcTreeOctant> new_octant_tnw;
        new_octant_tnw = root->prototype();
        new_octant_tnw->center = bsw_center + Vector3(0, root->span, root->span);
        new_octant_tnw->span = root->span/2;
        new_octant_tnw->parent = root.ptr();
        root->children[5] = new_octant_tnw;
        Ref<OcTreeOctant> new_octant_tse;
        new_octant_tse = root->prototype();
        new_octant_tse->center = bsw_center + Vector3(root->span, root->span, 0);
        new_octant_tse->span = root->span/2;
        new_octant_tse->parent = root.ptr();
        root->children[6] = new_octant_tse;
        Ref<OcTreeOctant> new_octant_tne;
        new_octant_tne = root->prototype();
        new_octant_tne->center = bsw_center + Vector3(root->span, root->span, root->span);
        new_octant_tne->span = root->span/2;
        new_octant_tne->parent = root.ptr();
        root->children[7] = new_octant_tne;
    }

    for(int64_t i = 0; i < root->children.size(); i++){
        if(Ref<OcTreeOctant>(root->children[i]).is_valid())
            split_recursive_on_cell(Ref<OcTreeOctant>(root->children[i]), condition);
    }

}



void OcTree::split_recursive_on_condition(Ref<OcTreeOctant> root, Callable condition){
    
    Ref<OcTreeOctant> current = root;
    uint64_t depth = 0;
    while(!(current->is_root())){
        depth++;
        current = Ref<OcTreeOctant>(current->parent);
    }

    if (depth >= maximum_depth)
        return;

    Variant result = condition.call();

    if(result.operator bool()) {
        root->children.resize(8);
        Vector3 bsw_center = root->center - Vector3(root->span/2, root->span/2, root->span/2);
        Ref<OcTreeOctant> new_octant_bsw;
        new_octant_bsw = root->prototype();
        new_octant_bsw->center = bsw_center;
        new_octant_bsw->span = root->span/2;
        new_octant_bsw->parent = root.ptr();
        root->children[0] = new_octant_bsw;
        Ref<OcTreeOctant> new_octant_bnw;
        new_octant_bnw = root->prototype();
        new_octant_bnw->center = bsw_center + Vector3(0, 0, root->span);
        new_octant_bnw->span = root->span/2;
        new_octant_bnw->parent = root.ptr();
        root->children[1] = new_octant_bnw;
        Ref<OcTreeOctant> new_octant_bse;
        new_octant_bse = root->prototype();
        new_octant_bse->center = bsw_center + Vector3(root->span, 0, 0);
        new_octant_bse->span = root->span/2;
        new_octant_bse->parent = root.ptr();
        root->children[2] = new_octant_bse;
        Ref<OcTreeOctant> new_octant_bne;
        new_octant_bne = root->prototype();
        new_octant_bne->center = bsw_center + Vector3(root->span, 0, root->span);
        new_octant_bne->span = root->span/2;
        new_octant_bne->parent = root.ptr();
        root->children[3] = new_octant_bne;
        Ref<OcTreeOctant> new_octant_tsw;
        new_octant_tsw = root->prototype();
        new_octant_tsw->center = bsw_center + Vector3(0, root->span, 0);
        new_octant_tsw->span = root->span/2;
        new_octant_tsw->parent = root.ptr();
        root->children[4] = new_octant_tsw;
        Ref<OcTreeOctant> new_octant_tnw;
        new_octant_tnw = root->prototype();
        new_octant_tnw->center = bsw_center + Vector3(0, root->span, root->span);
        new_octant_tnw->span = root->span/2;
        new_octant_tnw->parent = root.ptr();
        root->children[5] = new_octant_tnw;
        Ref<OcTreeOctant> new_octant_tse;
        new_octant_tse = root->prototype();
        new_octant_tse->center = bsw_center + Vector3(root->span, root->span, 0);
        new_octant_tse->span = root->span/2;
        new_octant_tse->parent = root.ptr();
        root->children[6] = new_octant_tse;
        Ref<OcTreeOctant> new_octant_tne;
        new_octant_tne = root->prototype();
        new_octant_tne->center = bsw_center + Vector3(root->span, root->span, root->span);
        new_octant_tne->span = root->span/2;
        new_octant_tne->parent = root.ptr();
        root->children[7] = new_octant_tne;
    }

    for(int64_t i = 0; i < root->children.size(); i++){
        if(Ref<OcTreeOctant>(root->children[i]).is_valid())
            split_recursive_on_condition(Ref<OcTreeOctant>(root->children[i]), condition);
    }

}

int64_t OcTree::find_depth(){
    TypedArray<OcTreeOctant> leaves = layout_leaves();

    if(leaves.is_empty())
        return 0;
    
    int64_t greatest_depth = 0;
    int64_t current_depth = 0;
    for(int64_t i = 0; i < leaves.size(); i++){
        current_depth = 0;
        Ref<OcTreeOctant> current_octant = leaves[i];
        while(!current_octant->is_root()){
            current_depth++;
            current_octant = Ref<OcTreeOctant>(current_octant->parent);
        }
        if(current_depth > greatest_depth)
            greatest_depth = current_depth;
    }

    return greatest_depth;
}

TypedArray<OcTreeOctant> OcTree::layout_leaves(){
    TypedArray<OcTreeOctant> leaves;
    layout_leaves_recursive(get_root(), leaves);
    return leaves;
}

void OcTree::layout_leaves_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> leaves){
    if(root->is_leaf())
        leaves.push_back(root);

    TypedArray<OcTreeOctant> root_children = root->children;

     for(int64_t i = 0; i < root_children.size(); i++){
        Ref<OcTreeOctant> root_child = root_children[i];
        layout_leaves_recursive(root_child, leaves);
    }
}

Ref<OcTreeOctant> OcTree::leaf_that_contains_point(Vector3 point){
    if(!get_root()->contains_point(point))
        return Ref<OcTreeOctant>();
    return leaf_that_contains_point_recursive(get_root(), point);
}

Ref<OcTreeOctant> OcTree::leaf_that_contains_point_recursive(Ref<OcTreeOctant> root, Vector3 point){
    Ref<OcTreeOctant> queried = root;
    while (!queried->is_leaf()){
        for(int64_t i = 0; i < queried->children.size(); i++){
            Ref<OcTreeOctant> queried_child = queried->children[i];
            if(queried_child->contains_point(point)){
                queried = queried_child;
                break;
            }
        }
    }
    return queried;
}

Ref<OcTreeOctant> OcTree::cell_that_contains_point(Vector3 point, int64_t at_depth){
    if(!get_root()->contains_point(point))
        return Ref<OcTreeOctant>();
    return cell_that_contains_point_recursive(get_root(), point, at_depth);
}

Ref<OcTreeOctant> OcTree::cell_that_contains_point_recursive(Ref<OcTreeOctant> root, Vector3 point, int64_t at_depth){
    Ref<OcTreeOctant> queried = root;
    int64_t depth = 0;
    while (depth < at_depth){
        for(int64_t i = 0; i < queried->children.size(); i++){
            Ref<OcTreeOctant> queried_child = queried->children[i];
            if(queried_child->contains_point(point)){
                queried = queried_child;
                depth++;
                break;
            }
        }
    }
    return queried;
}

TypedArray<OcTreeOctant> OcTree::cells_that_intersect_ray(Vector3 ray_origin, Vector3 ray_direction){
    TypedArray<OcTreeOctant> arr;
    cells_that_intersect_ray_recursive(get_root(), arr, ray_origin, ray_direction);
    return arr;
}


void OcTree::cells_that_intersect_ray_recursive(Ref<OcTreeOctant> root, TypedArray<OcTreeOctant> &intersected, Vector3 ray_origin, Vector3 ray_direction){
    if(root->slab_test(ray_origin, ray_direction)){
        if(root->is_leaf())
            intersected.push_back(root);
    
        for(int64_t i = 0; i < root->children.size(); i++)
            cells_that_intersect_ray_recursive(root->children[i], intersected, ray_origin, ray_direction);
    }
}



Ref<OcTreeOctant> OcTree::find_neighbor_for_traversal(Ref<OcTreeOctant> octant, int64_t neighbor){
    Ref<OcTreeOctant> query = octant;
    Ref<OcTreeOctant> neighbor_query = find_neighbor_of_equal_depth(query, neighbor);
    while(neighbor_query.is_null()){
        query = Ref<OcTreeOctant>(query->parent);
        if(query->is_root())
            return Ref<OcTreeOctant>();
        neighbor_query = find_neighbor_of_equal_depth(query, neighbor);
    }
    return neighbor_query;
}

Ref<OcTreeOctant> OcTree::find_neighbor_of_equal_depth(Ref<OcTreeOctant> octant, int64_t neighbor){
    if(octant->is_root())
        return Ref<OcTreeOctant>();
    Ref<OcTreeOctant> parent_ref = Ref<OcTreeOctant>(octant->parent);
    // Ref<OcTreeOctant> parent_ref = Ref<OcTreeOctant>(Object::cast_to<OcTreeOctant>(octant->parent));
    OCTANT_POSITION selected_octant_position = (OCTANT_POSITION) (parent_ref->children.find(octant));
    int64_t n_query = octree_eqd_neighbor_table[selected_octant_position][neighbor][0];

    if(n_query == -1){
        return parent_ref->children[octree_eqd_neighbor_table[selected_octant_position][neighbor][1]];
    }else{
         Ref<OcTreeOctant> parent_neighbor = 
            find_neighbor_of_equal_depth(Ref<OcTreeOctant>(octant->parent), n_query);
        if(parent_neighbor.is_null())
            return Ref<OcTreeOctant>();
        return parent_neighbor->children[octree_eqd_neighbor_table[selected_octant_position][neighbor][1]];
    }
}

TypedArray<OcTreeOctant> OcTree::find_neighbors_of_equal_depth(Ref<OcTreeOctant> octant, PackedInt64Array neighbors){
    if(octant->is_root())
        return TypedArray<OcTreeOctant>();

    TypedArray<OcTreeOctant> arr;

    for(int64_t i = 0; i < neighbors.size(); i++){
        arr.push_back(find_neighbor_of_equal_depth(octant, (((int64_t) neighbors[i]))));
    }

    return arr;
}

TypedArray<OcTreeOctant> OcTree::find_leaf_neighbors(Ref<OcTreeOctant> octant, int64_t neighbor){
    Ref<OcTreeOctant> base_neighbor = find_neighbor_of_equal_depth(octant, neighbor);
    Ref<OcTreeOctant> position = base_neighbor;
    Ref<OcTreeOctant> base_position = base_neighbor;
    
    TypedArray<OcTreeOctant> arr;
    
    switch (neighbor){
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_NORTH_EAST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_BELOW_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_SOUTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_NORTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_SOUTH_EAST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_SOUTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_EAST);
            }
        case OCTANT_NEIGHBOR::OCTANT_BELOW:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::TOP_SOUTH_WEST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_EAST);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_NORTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_NORTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_WEST);
            }
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_NORTH_WEST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_BELOW_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_NORTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_SOUTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::TOP_SOUTH_WEST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_NORTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_WEST:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::BOTTOM_SOUTH_EAST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_NORTH);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_NORTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_NORTH:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::BOTTOM_SOUTH_WEST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_EAST);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_SOUTH:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::BOTTOM_NORTH_EAST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_WEST);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[BOTTOM_NORTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_EAST:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::BOTTOM_NORTH_WEST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_SOUTH);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_NORTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_ABOVE);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_NORTH_EAST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_NORTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_WEST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_EAST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_EAST);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE:
            while(!(base_position->is_leaf()))
                base_position = base_position->children[OCTANT_POSITION::BOTTOM_SOUTH_WEST];
            while(base_position->is_descendant_of(base_neighbor)){
                position = base_position;
                while(position->is_descendant_of(base_neighbor)){
                    arr.push_back(position);
                    position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_EAST);
                }
                base_position = find_neighbor_for_traversal(base_position, OCTANT_NEIGHBOR::OCTANT_NORTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_NORTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_WEST);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_NORTH_WEST];
            arr.push_back(position);
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_NORTH_WEST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, OCTANT_NEIGHBOR::OCTANT_SOUTH);
            }
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_EAST:
            while(!(position->is_leaf()))
                position = position->children[OCTANT_POSITION::BOTTOM_SOUTH_WEST];
            arr.push_back(position);
    }
    return arr;
}

Array OcTree::gen_arrays(){
    PackedVector3Array verts;
    PackedInt32Array indices;
    gen_arrays_recursive(root, verts, indices);
    Array arr;
    arr.resize(Mesh::ARRAY_MAX);
    arr[Mesh::ARRAY_VERTEX] = verts;
    arr[Mesh::ARRAY_INDEX] = indices;
    return arr;
}

void OcTree::gen_arrays_recursive(Ref<OcTreeOctant> root, PackedVector3Array &vertices, PackedInt32Array &indices){
    int depth = vertices.size()/8;

   
    Vector3 bsw_corner = root->center - Vector3(root->span, root->span, root->span);
    vertices.push_back(bsw_corner);
    vertices.push_back(bsw_corner + Vector3(0, 0, 2*root->span));
    vertices.push_back(bsw_corner + Vector3(2*root->span, 0, 2*root->span));
    vertices.push_back(bsw_corner + Vector3(2*root->span, 0, 0));
    vertices.push_back(bsw_corner + Vector3(0, 2*root->span, 0));
    vertices.push_back(bsw_corner + Vector3(0, 2*root->span, 2*root->span));
    vertices.push_back(bsw_corner + Vector3(2*root->span, 2*root->span, 2*root->span));
    vertices.push_back(bsw_corner + Vector3(2*root->span, 2*root->span, 0));

    //BOTTOM FACE EDGES
	indices.push_back(0 + depth * 8);
	indices.push_back(1 + depth * 8);
	indices.push_back(1 + depth * 8);
	indices.push_back(2 + depth * 8);
	indices.push_back(2 + depth * 8);
	indices.push_back(3 + depth * 8);
	indices.push_back(3 + depth * 8);
	indices.push_back(0 + depth * 8);
	//TOP FACE EDGES
	indices.push_back(4 + depth * 8);
	indices.push_back(5 + depth * 8);
	indices.push_back(5 + depth * 8);
	indices.push_back(6 + depth * 8);
	indices.push_back(6 + depth * 8);
	indices.push_back(7 + depth * 8);
	indices.push_back(7 + depth * 8);
	indices.push_back(4 + depth * 8);
	//BETWEEN EDGES
	indices.push_back(0 + depth * 8);
	indices.push_back(4 + depth * 8);
	indices.push_back(1 + depth * 8);
	indices.push_back(5 + depth * 8);
	indices.push_back(2 + depth * 8);
	indices.push_back(6 + depth * 8);
	indices.push_back(3 + depth * 8);
	indices.push_back(7 + depth * 8);

    for(int64_t i = 0; i < root->children.size(); i++){
        if(Ref<OcTreeOctant>(root->children[i]).is_valid())
            gen_arrays_recursive(Ref<OcTreeOctant>(root->children[i]), vertices, indices);
    }


}
