#include "quadtree.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;





void QuadTree::_bind_methods(){
    ClassDB::bind_method(D_METHOD("flatten"), &QuadTree::flatten);
    ClassDB::bind_static_method("QuadTree", D_METHOD("json_serialize", "quadtree"), &QuadTree::json_serialize);
    ClassDB::bind_static_method("QuadTree", D_METHOD("json_deserialize", "dictionary_array"), &QuadTree::json_deserialize);
    ClassDB::bind_method(D_METHOD("cell_count"), &QuadTree::cell_count);
    ClassDB::bind_method(D_METHOD("split_on_cell", "condition_on_cell"), &QuadTree::split_on_cell);
	ClassDB::bind_method(D_METHOD("split_on_condition", "condition"), &QuadTree::split_on_condition);
    ClassDB::bind_method(D_METHOD("find_depth"), &QuadTree::find_depth);
    ClassDB::bind_method(D_METHOD("layout_leaves"), &QuadTree::layout_leaves);
    ClassDB::bind_method(D_METHOD("layout_level", "at_depth"), &QuadTree::layout_level);
	ClassDB::bind_method(D_METHOD("leaf_that_contains_point", "point"), &QuadTree::leaf_that_contains_point);
	ClassDB::bind_method(D_METHOD("cell_that_contains_point", "point", "at_depth"), &QuadTree::cell_that_contains_point);
    ClassDB::bind_method(D_METHOD("leaves_in_radius", "center", "radius"), &QuadTree::leaves_in_radius);
	ClassDB::bind_method(D_METHOD("leaves_in_bounding_box", "center", "span"), &QuadTree::leaves_in_bounding_box);
	ClassDB::bind_method(D_METHOD("cells_in_radius", "center", "radius", "at_depth"), &QuadTree::cells_in_radius);
	ClassDB::bind_method(D_METHOD("cells_in_bounding_box", "center", "span", "at_depth"), &QuadTree::cells_in_bounding_box);
	ClassDB::bind_method(D_METHOD("cells_that_intersect_ray", "ray_origin", "ray_direction"), &QuadTree::cells_that_intersect_ray);
    ClassDB::bind_method(D_METHOD("find_neighbor_for_traversal", "quadrant", "neighbor"), &QuadTree::find_neighbor_for_traversal);
    ClassDB::bind_method(D_METHOD("find_neighbor_of_equal_depth", "quadrant", "neighbor"), &QuadTree::find_neighbor_of_equal_depth);
	ClassDB::bind_method(D_METHOD("find_neighbors_of_equal_depth", "quadrant", "neighbors"), &QuadTree::find_neighbors_of_equal_depth);
    ClassDB::bind_method(D_METHOD("find_leaf_neighbors", "quadrant", "neighbor"), &QuadTree::find_leaf_neighbors);
	ClassDB::bind_method(D_METHOD("gen_lines"), &QuadTree::gen_lines);

    ClassDB::bind_method(D_METHOD("set_root", "root"), &QuadTree::set_root);
    ClassDB::bind_method(D_METHOD("get_root"), &QuadTree::get_root);
	ClassDB::bind_method(D_METHOD("set_maximum_depth", "maximum_depth"), &QuadTree::set_maximum_depth);
    ClassDB::bind_method(D_METHOD("get_maximum_depth"), &QuadTree::get_maximum_depth);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "maximum_depth"), "set_maximum_depth", "get_maximum_depth");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root"), "set_root", "get_root");



    BIND_ENUM_CONSTANT(QUADRANT_SOUTH_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH);
    BIND_ENUM_CONSTANT(QUADRANT_SOUTH);
    BIND_ENUM_CONSTANT(QUADRANT_SOUTH_EAST);
    BIND_ENUM_CONSTANT(QUADRANT_EAST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH_EAST);
    BIND_ENUM_CONSTANT(QUADRANT_NEIGHBOR_MAX);

    //VIRTUAL METHODS
    GDVIRTUAL_BIND(_generate_meta_dictionary);

}


QuadTree::QuadTree(){
}
QuadTree::~QuadTree(){
}


TypedArray<FlatQuadTreeQuadrant> QuadTree::flatten(){
    TypedArray<FlatQuadTreeQuadrant> res;
    TypedArray<QuadTreeQuadrant> linear = linearize();
    
    res.resize(linear.size());
    Ref<FlatQuadTreeQuadrant> input;
    Ref<QuadTreeQuadrant> match;
    for(int64_t i = 0; i < linear.size(); i++){
        match = linear[i];
        input = match->flatten();
        if(!match->is_root()){
            input->parent = -1;
            for(int64_t j = 0; j < linear.size(); j++){
                Ref<QuadTreeQuadrant> query = linear[j];
                if(query.ptr() == match->parent){
                    input->parent = j;
                    break;
                }
            }
        }
        if(!match->is_leaf())
            for(int64_t j = 0; j < 4; j++)
                input->children[j] = linear.find(match->children[j]);

        res[i] = input;
    }
    return res;
}


TypedArray<QuadTreeQuadrant> QuadTree::linearize(){
    TypedArray<QuadTreeQuadrant> res;
    linearize_recursive(get_root(), res);
    return res;
}
void QuadTree::linearize_recursive(Ref<QuadTreeQuadrant> _root, TypedArray<QuadTreeQuadrant> &arr){

    arr.push_back(_root);

    for(int64_t i = 0; i < _root->children.size(); i++)
        if(Ref<QuadTreeQuadrant>(root->children[i]).is_valid())
            linearize_recursive(_root->children[i], arr);

}

TypedArray<QuadTreeQuadrant> QuadTree::flat_to_linear(TypedArray<FlatQuadTreeQuadrant> flat_array){
    TypedArray<QuadTreeQuadrant> res;
    flat_to_linear_recursive(flat_array, res, 0, 0);
    return res;
}

void QuadTree::flat_to_linear_recursive(TypedArray<FlatQuadTreeQuadrant> flat_array, TypedArray<QuadTreeQuadrant> &linear_array, int64_t idx, int64_t child_idx){

    Ref<FlatQuadTreeQuadrant> flat_quadrant = flat_array[idx];
    Ref<QuadTreeQuadrant> new_quadrant = flat_quadrant->unflatten();

    //If quadrant is not root
    if(flat_quadrant->parent != -1){
        //Associate parent
        new_quadrant->parent = Object::cast_to<QuadTreeQuadrant>(linear_array[flat_quadrant->parent]);
        Ref<QuadTreeQuadrant> parent_ref = Ref<QuadTreeQuadrant>(new_quadrant->parent);
        // Ref<QuadTreeQuadrant> parent_ref = Ref<QuadTreeQuadrant>(Object::cast_to<QuadTreeQuadrant>(new_quadrant->parent));
        //If parent hasn't been touched, resize
        if(parent_ref->children.size() != 4)
           parent_ref->children.resize(4);
        //Add as child
        parent_ref->children[child_idx] = new_quadrant;
    }

    linear_array.push_back(new_quadrant);

    for(int64_t i = 0; i < flat_quadrant->children.size(); i++)
        if(flat_quadrant->children[i] != -1)
            flat_to_linear_recursive(flat_array, linear_array, flat_quadrant->children[i], i);

}

Dictionary QuadTree::generate_meta_dictionary(){
    Dictionary meta;
    if(GDVIRTUAL_IS_OVERRIDDEN(_generate_meta_dictionary)){
        _gdvirtual__generate_meta_dictionary_call<false>(meta);
    }else{
        meta.get_or_add("maximum_depth", maximum_depth);
    }
    return meta;
}


TypedArray<Dictionary> QuadTree::json_serialize(Ref<QuadTree> quadtree){
    TypedArray<Dictionary> res;
    
    TypedArray<FlatQuadTreeQuadrant> flat_array = quadtree->flatten();

    Dictionary meta_dictionary = quadtree->generate_meta_dictionary();
    res.resize(flat_array.size() + 1);
    res[0] = meta_dictionary;

    Ref<FlatQuadTreeQuadrant> flat_quadrant;
    for(int64_t i = 0; i < flat_array.size(); i++){
        flat_quadrant = flat_array[i];
        res[i+1] = flat_quadrant->to_dict();
    }
    return res;
}
Ref<godot::QuadTree> QuadTree::json_deserialize(TypedArray<Dictionary> data){
    if(!data[0].has_key("maximum_depth")){
        //print err
        return Ref<godot::QuadTree>();
    }
    
    TypedArray<FlatQuadTreeQuadrant> flat_array;
    flat_array.resize(data.size()-1);
    Ref<FlatQuadTreeQuadrant> flat_quadrant;
    Dictionary current_dictionary;
    for(int64_t i = 1; i < data.size(); i++){
        current_dictionary = data[i];
        flat_quadrant.instantiate();
        flat_quadrant->copy_from_dict(current_dictionary);
        flat_array[i-1] = flat_quadrant;
    }
    
    TypedArray<QuadTreeQuadrant> linear = flat_to_linear(flat_array);

    Dictionary meta = data[0];
    Ref<godot::QuadTree> res;
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

Ref<QuadTreeQuadrant> QuadTree::get_root(){
    return Ref<QuadTreeQuadrant>(root);
}
void QuadTree::set_root(Ref<QuadTreeQuadrant> _root){
    root = _root;
}
int64_t QuadTree::get_maximum_depth(){
    return maximum_depth;
}
void QuadTree::set_maximum_depth(int64_t p_maximum_depth){
    maximum_depth = p_maximum_depth;
}

int64_t QuadTree::cell_count(){
    int64_t ct = 0;
    cell_count_recursive(root, ct);
    return ct;
}
void QuadTree::cell_count_recursive(Ref<QuadTreeQuadrant> root, int64_t &ct){
    if(root.is_null())
        return;
    ct++;
    for(int64_t i = 0; i < root->children.size(); i++)
        cell_count_recursive(root->children[i], ct);
}

void QuadTree::split_on_cell(Callable condition_on_cell){
    split_recursive_on_cell(root, condition_on_cell);
}

void QuadTree::split_on_condition(Callable condition){
    split_recursive_on_condition(root, condition);
}

void QuadTree::split_recursive_on_cell(Ref<QuadTreeQuadrant> root, Callable condition){

    Ref<QuadTreeQuadrant> current = root;
    uint64_t depth = 0;
    while(!(current->is_root())){
        depth++;
        current = Ref<QuadTreeQuadrant>(current->parent);
        
    }

    if (depth >= maximum_depth)
        return;

    Variant result = condition.call(root);

    if(result.operator bool()) {
        root->children.resize(4);
        Vector2 sw_center = root->center - Vector2(root->span/2, root->span/2);
        Ref<QuadTreeQuadrant> quadrant_sw;
        quadrant_sw = root->prototype();
        quadrant_sw->center = sw_center;
        quadrant_sw->span = root->span/2;
        quadrant_sw->parent = root.ptr();
        root->children[0] = quadrant_sw;
        Ref<QuadTreeQuadrant> quadrant_nw;
        quadrant_nw = root->prototype();
        quadrant_nw->center = sw_center + Vector2(0, root->span);
        quadrant_nw->span = root->span/2;
        quadrant_nw->parent = root.ptr();
        root->children[1] = quadrant_nw;
        Ref<QuadTreeQuadrant> quadrant_se;
        quadrant_se = root->prototype();
        quadrant_se->center = sw_center + Vector2(root->span, 0);
        quadrant_se->span = root->span/2;
        quadrant_se->parent = root.ptr();
        root->children[2] = quadrant_se;
        Ref<QuadTreeQuadrant> quadrant_ne;
        quadrant_ne = root->prototype();
        quadrant_ne->center = sw_center + Vector2(root->span, root->span);
        quadrant_ne->span = root->span/2;
        quadrant_ne->parent = root.ptr();
        root->children[3] = quadrant_ne;
    }

    for(int64_t i = 0; i < root->children.size(); i++){
        if(Ref<QuadTreeQuadrant>(root->children[i]).is_valid())
            split_recursive_on_cell(Ref<QuadTreeQuadrant>(root->children[i]), condition);
    }


}


void QuadTree::split_recursive_on_condition(Ref<QuadTreeQuadrant> root, Callable condition){
    Ref<QuadTreeQuadrant> current = root;
    uint64_t depth = 0;
    while(!(current->is_root())){
        depth++;
        current = Ref<QuadTreeQuadrant>(current->parent);
    }
    
    if (depth >= maximum_depth)
        return;
    
    Variant result = condition.call();
    if(result.operator bool()){
        root->children.resize(4);
        Vector2 sw_center = root->center - Vector2(root->span/2, root->span/2);
        Ref<QuadTreeQuadrant> quadrant_sw;
        quadrant_sw = root->prototype();
        quadrant_sw->center = sw_center;
        quadrant_sw->span = root->span/2;
        quadrant_sw->parent = root.ptr();
        root->children[0] = quadrant_sw;
        Ref<QuadTreeQuadrant> quadrant_nw;
        quadrant_nw = root->prototype();
        quadrant_nw->center = sw_center + Vector2(0, root->span);
        quadrant_nw->span = root->span/2;
        quadrant_nw->parent = root.ptr();
        root->children[1] = quadrant_nw;
        Ref<QuadTreeQuadrant> quadrant_se;
        quadrant_se = root->prototype();
        quadrant_se->center = sw_center + Vector2(root->span, 0);
        quadrant_se->span = root->span/2;
        quadrant_se->parent = root.ptr();
        root->children[2] = quadrant_se;
        Ref<QuadTreeQuadrant> quadrant_ne;
        quadrant_ne = root->prototype();
        quadrant_ne->center = sw_center + Vector2(root->span, root->span);
        quadrant_ne->span = root->span/2;
        quadrant_ne->parent = root.ptr();
        root->children[3] = quadrant_ne;
    }

    for(int64_t i = 0; i < root->children.size(); i++){
        if(Ref<QuadTreeQuadrant>(root->children[i]).is_valid())
            split_recursive_on_condition(Ref<QuadTreeQuadrant>(root->children[i]), condition);
    }

}

int64_t QuadTree::find_depth(){
    TypedArray<QuadTreeQuadrant> leaves = layout_leaves();

    if(leaves.is_empty())
        return 0;
    
    int64_t greatest_depth = 0;
    int64_t current_depth = 0;
    for(int64_t i = 0; i < leaves.size(); i++){
        current_depth = 0;
        Ref<QuadTreeQuadrant> current_quadrant = leaves[i];
        while(!current_quadrant->is_root()){
            current_depth++;
            current_quadrant = Ref<QuadTreeQuadrant>(current_quadrant->parent);
        }
        if(current_depth > greatest_depth)
            greatest_depth = current_depth;
    }

    return greatest_depth;
}

TypedArray<QuadTreeQuadrant> QuadTree::layout_leaves(){
    TypedArray<QuadTreeQuadrant> leaves;
    layout_leaves_recursive(get_root(), leaves);
    return leaves;
}

void QuadTree::layout_leaves_recursive(Ref<QuadTreeQuadrant> root, TypedArray<QuadTreeQuadrant>& leaves){
    if(root->is_leaf())
        leaves.push_back(root);

    TypedArray<QuadTreeQuadrant> root_children = root->children;

     for(int64_t i = 0; i < root_children.size(); i++){
        Ref<QuadTreeQuadrant> root_child = root_children[i];
        layout_leaves_recursive(root_child, leaves);
    }
}

TypedArray<QuadTreeQuadrant> QuadTree::layout_level(int64_t at_depth){
    TypedArray<QuadTreeQuadrant> arr;
    layout_level_recursive(get_root(), at_depth, arr);
    return arr;
}
void QuadTree::layout_level_recursive(Ref<QuadTreeQuadrant> root, int64_t at_depth, TypedArray<QuadTreeQuadrant>& arr){
    Ref<QuadTreeQuadrant> current = root;
    uint64_t depth = 0;
    while(!(current->is_root())){
        depth++;
        current = Ref<QuadTreeQuadrant>(current->parent);
    }

    if (depth == at_depth)
        arr.push_back(root);
        return;
    
    TypedArray<QuadTreeQuadrant> root_children = root->children;

    for(int64_t i = 0; i < root_children.size(); i++){
        Ref<QuadTreeQuadrant> root_child = root_children[i];
        layout_level_recursive(root_child, at_depth, arr);
    }
}

Ref<QuadTreeQuadrant> QuadTree::leaf_that_contains_point(Vector2 point){
    if(!get_root()->contains_point(point))
        return Ref<QuadTreeQuadrant>();
    return leaf_that_contains_point_recursive(get_root(), point);
}
Ref<QuadTreeQuadrant> QuadTree::leaf_that_contains_point_recursive(Ref<QuadTreeQuadrant> root, Vector2 point){
    Ref<QuadTreeQuadrant> queried = root;
    while (!queried->is_leaf()){
        for(int64_t i = 0; i < queried->children.size(); i++){
            Ref<QuadTreeQuadrant> queried_child = queried->children[i];
            if(queried_child->contains_point(point)){
                queried = queried_child;
                break;
            }
        }
    }
    return queried;
}

Ref<QuadTreeQuadrant> QuadTree::cell_that_contains_point(Vector2 point, int64_t at_depth){
    if(!get_root()->contains_point(point))
        return Ref<QuadTreeQuadrant>();
    return cell_that_contains_point_recursive(get_root(), point, at_depth);
}
Ref<QuadTreeQuadrant> QuadTree::cell_that_contains_point_recursive(Ref<QuadTreeQuadrant> root, Vector2 point, int64_t at_depth){
    Ref<QuadTreeQuadrant> queried = root;
    int64_t depth = 0;
    while (depth < at_depth){
        for(int64_t i = 0; i < queried->children.size(); i++){
            Ref<QuadTreeQuadrant> queried_child = queried->children[i];
            if(queried_child->contains_point(point)){
                queried = queried_child;
                depth++;
                break;
            }
        }
    }
    return queried;
}

TypedArray<QuadTreeQuadrant> QuadTree::leaves_in_radius(Vector2 center, real_t radius){
    TypedArray<QuadTreeQuadrant> res;
    
    TypedArray<QuadTreeQuadrant> leaves = layout_leaves();

    for(int64_t i = 0; i < leaves.size(); i++){
        Ref<QuadTreeQuadrant> cell = leaves[i];
        if(cell->center.distance_to(center) <= radius){
            res.push_back(cell);
        }
    }

    return res;
}
TypedArray<QuadTreeQuadrant> QuadTree::leaves_in_bounding_box(Vector2 center, real_t span){
    TypedArray<QuadTreeQuadrant> res;
    
    TypedArray<QuadTreeQuadrant> leaves = layout_leaves();

    for(int64_t i = 0; i < leaves.size(); i++){
        Ref<QuadTreeQuadrant> cell = leaves[i];
        if(cell->center.x - cell->span >= center.x - span &&
            cell->center.x + cell->span <= center.x + span &&
            cell->center.y - cell->span >= center.y - span &&
            cell->center.y + cell->span <= center.y + span){
                res.push_back(cell);
        }
    }
    
    return res;
}


TypedArray<QuadTreeQuadrant> QuadTree::cells_in_radius(Vector2 center, real_t radius, int64_t at_depth){
    TypedArray<QuadTreeQuadrant> res;
    
    TypedArray<QuadTreeQuadrant> eqd_cells = layout_level(at_depth);

    for(int64_t i = 0; i < eqd_cells.size(); i++){
        Ref<QuadTreeQuadrant> cell = eqd_cells[i];
        if(cell->center.distance_to(center) <= radius){
            res.push_back(cell);
        }
    }

    return res;
}
TypedArray<QuadTreeQuadrant> QuadTree::cells_in_bounding_box(Vector2 center, real_t span, int64_t at_depth){
    TypedArray<QuadTreeQuadrant> res;
    
    TypedArray<QuadTreeQuadrant> eqd_cells = layout_level(at_depth);

    for(int64_t i = 0; i < eqd_cells.size(); i++){
        Ref<QuadTreeQuadrant> cell = eqd_cells[i];
        if(cell->center.x - cell->span >= center.x - span &&
            cell->center.x + cell->span <= center.x + span &&
            cell->center.y - cell->span >= center.y - span &&
            cell->center.y + cell->span <= center.y + span){
                res.push_back(cell);
        }
    }
    
    return res;
}

TypedArray<QuadTreeQuadrant> QuadTree::cells_that_intersect_ray(Vector2 ray_origin, Vector2 ray_direction){
    TypedArray<QuadTreeQuadrant> arr;
    cells_that_intersect_ray_recursive(get_root(), arr, ray_origin, ray_direction);
    return arr;
}


void QuadTree::cells_that_intersect_ray_recursive(Ref<QuadTreeQuadrant> root, TypedArray<QuadTreeQuadrant> &intersected, Vector2 ray_origin, Vector2 ray_direction){
    if(root->slab_test(ray_origin, ray_direction)){
        if(root->is_leaf())
            intersected.push_back(root);
    
        for(int64_t i = 0; i < root->children.size(); i++)
            cells_that_intersect_ray_recursive(root->children[i], intersected, ray_origin, ray_direction);
    }
}


Ref<QuadTreeQuadrant> QuadTree::find_neighbor_for_traversal(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor){
    Ref<QuadTreeQuadrant> query = quadrant;
    Ref<QuadTreeQuadrant> neighbor_query = find_neighbor_of_equal_depth(query, neighbor);
    while(neighbor_query.is_null()){
        query = Ref<QuadTreeQuadrant>(query->parent);
        if(query->is_root())
            return Ref<QuadTreeQuadrant>();
        neighbor_query = find_neighbor_of_equal_depth(query, neighbor);
    }
    return neighbor_query;
}


Ref<QuadTreeQuadrant> QuadTree::find_neighbor_of_equal_depth(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor){
    if(quadrant->is_root())
        return Ref<QuadTreeQuadrant>();

    Ref<QuadTreeQuadrant> parent_ref = Ref<QuadTreeQuadrant>(quadrant->parent);
    // Ref<QuadTreeQuadrant> parent_ref = Ref<QuadTreeQuadrant>(Object::cast_to<QuadTreeQuadrant>(quadrant->parent));
    QUADRANT_POSITION selected_quadrant_position = (QUADRANT_POSITION) parent_ref->children.find(quadrant);

    int64_t n_query = quadtree_eqd_neighbor_table[selected_quadrant_position][neighbor][0];

    if(n_query == -1){
        return parent_ref->children[quadtree_eqd_neighbor_table[selected_quadrant_position][neighbor][1]];
    }else{
         Ref<QuadTreeQuadrant> parent_neighbor = 
            find_neighbor_of_equal_depth(parent_ref, n_query);
        if(parent_neighbor.is_null())
            return Ref<QuadTreeQuadrant>();
        return parent_neighbor->children[quadtree_eqd_neighbor_table[selected_quadrant_position][neighbor][1]];
    }
}

TypedArray<QuadTreeQuadrant> QuadTree::find_neighbors_of_equal_depth(Ref<QuadTreeQuadrant> quadrant, PackedInt64Array neighbors){
    if(quadrant->is_root())
        return TypedArray<QuadTreeQuadrant>();
    
    TypedArray<QuadTreeQuadrant> arr;
    arr.resize(neighbors.size());
    for(int64_t i = 0; i < neighbors.size(); i++){
        arr[i] = find_neighbor_of_equal_depth(quadrant, (QUADRANT_NEIGHBOR)((int64_t) neighbors[i]));
    }
    return arr;
}

TypedArray<QuadTreeQuadrant> QuadTree::find_leaf_neighbors(Ref<QuadTreeQuadrant> quadrant, int64_t neighbor){
    Ref<QuadTreeQuadrant> base_neighbor = find_neighbor_of_equal_depth(quadrant, neighbor);
    Ref<QuadTreeQuadrant> position = base_neighbor;
    
    TypedArray<QuadTreeQuadrant> arr;

    switch (neighbor){
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_WEST:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::NORTH_EAST];
            }
            arr.push_back(position);
        case QUADRANT_NEIGHBOR::QUADRANT_WEST:
            position = base_neighbor;
            while(!position->is_leaf())
                position = position->children[QUADRANT_POSITION::SOUTH_EAST];
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, QUADRANT_NEIGHBOR::QUADRANT_NORTH);
            }
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_WEST:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::SOUTH_EAST];
            }
            arr.push_back(position);
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::SOUTH_WEST];
            }
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, QUADRANT_NEIGHBOR::QUADRANT_EAST);
            }
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::NORTH_EAST];
            }
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, QUADRANT_NEIGHBOR::QUADRANT_WEST);
            }
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_EAST:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::NORTH_WEST];
            }
            arr.push_back(position);
        case QUADRANT_NEIGHBOR::QUADRANT_EAST:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::NORTH_WEST];
            }
            while(position->is_descendant_of(base_neighbor)){
                arr.push_back(position);
                position = find_neighbor_for_traversal(position, QUADRANT_NEIGHBOR::QUADRANT_SOUTH);
            }
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_EAST:
            while(!position->is_leaf()){
                position = position->children[QUADRANT_POSITION::SOUTH_WEST];
            }
            arr.push_back(position);
    }
    return arr;
}


PackedVector2Array QuadTree::gen_lines(){
    PackedVector2Array arr;
    gen_lines_recursive(root, arr);
    return arr;
}

void QuadTree::gen_lines_recursive(Ref<QuadTreeQuadrant> root, PackedVector2Array &array){

    Vector2 sw_corner = root->center - Vector2(root->span, root->span);
    array.push_back(sw_corner);
    array.push_back(sw_corner + Vector2(0, 2*root->span));
    array.push_back(sw_corner + Vector2(0, 2*root->span));
    array.push_back(sw_corner + Vector2(2*root->span, 2*root->span));
    array.push_back(sw_corner + Vector2(2*root->span, 2*root->span));
    array.push_back(sw_corner + Vector2(2*root->span, 0));
    array.push_back(sw_corner + Vector2(2*root->span, 0));
    array.push_back(sw_corner);

    for(int64_t i = 0; i < root->children.size(); i++){
        gen_lines_recursive(Ref<QuadTreeQuadrant>(root->children[i]), array);
    }

}