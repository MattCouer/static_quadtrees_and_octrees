#include "z_quadtree.h"

using namespace godot;

void ZQuadTree::_bind_methods(){

    ClassDB::bind_static_method("ZQuadTree", D_METHOD("create", "total_depth", "initial_quadrant_center", "initial_quadrant_span"), &ZQuadTree::create);   
    ClassDB::bind_method(D_METHOD("find_depth"), &ZQuadTree::find_depth);   
    ClassDB::bind_method(D_METHOD("layout_leaves"), &ZQuadTree::layout_leaves);
	ClassDB::bind_method(D_METHOD("leaf_that_contains_point", "point"), &ZQuadTree::leaf_that_contains_point);
    ClassDB::bind_method(D_METHOD("get_quadrant_center", "quadrant"), &ZQuadTree::get_quadrant_center);
    ClassDB::bind_method(D_METHOD("get_quadrant_span", "depth"), &ZQuadTree::get_quadrant_span);
    ClassDB::bind_method(D_METHOD("get_quadrant_parent", "quadrant"), &ZQuadTree::get_quadrant_parent);
    ClassDB::bind_method(D_METHOD("get_quadrant_children", "quadrant"), &ZQuadTree::get_quadrant_children);
    ClassDB::bind_method(D_METHOD("find_neighbor_of_equal_depth", "quadrant", "neighbor"), &ZQuadTree::find_neighbor_of_equal_depth);
	ClassDB::bind_method(D_METHOD("find_neighbors_of_equal_depth", "quadrant", "neighbors"), &ZQuadTree::find_neighbors_of_equal_depth);
    ClassDB::bind_method(D_METHOD("find_leaf_neighbors", "quadrant", "neighbor"), &ZQuadTree::find_leaf_neighbors);
    ClassDB::bind_method(D_METHOD("split", "_total_depth"), &ZQuadTree::split);
	ClassDB::bind_method(D_METHOD("readjust", "_initial_quadrant_center", "_initial_quadrant_span"), &ZQuadTree::readjust);
	ClassDB::bind_method(D_METHOD("gen_lines", "at_depth"), &ZQuadTree::gen_lines);
	ClassDB::bind_method(D_METHOD("gen_z_lines", "at_depth"), &ZQuadTree::gen_z_lines);




    BIND_ENUM_CONSTANT(QUADRANT_SOUTH_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH_WEST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH);
    BIND_ENUM_CONSTANT(QUADRANT_SOUTH);
    BIND_ENUM_CONSTANT(QUADRANT_SOUTH_EAST);
    BIND_ENUM_CONSTANT(QUADRANT_EAST);
    BIND_ENUM_CONSTANT(QUADRANT_NORTH_EAST);

}

ZQuadTree::ZQuadTree(){

}
ZQuadTree::~ZQuadTree(){

}

Ref<ZQuadTree> ZQuadTree::create(int64_t _total_depth, Vector2 _initial_quadrant_center, float _initial_quadrant_span){
    Ref<ZQuadTree> res;
    res.instantiate();
    res->total_depth = _total_depth;
    res->initial_quadrant_center = _initial_quadrant_center;
    res->initial_quadrant_span = _initial_quadrant_span;
    return res;
}



int64_t ZQuadTree::cut_and_dilate(int64_t x){
    x = abs(x);
    // x is at this point 63 bits of whatever:          x == 0??? ???? ???? ???? ???? ???? ???? ???? ???? Z??? ???? ???? ???? ???? ???? ???A
    x &= 0b000000000000000000000000000000000001111111111111111111111111111;
    // now 36 bits of nothing and 28 bits of whatever:  x == 0000 0000 0000 0000 0000 0000 0000 0000 0000 Z??? ???? ???? ???? ???? ???? ???A
    x = (x ^ (x << 16)) & 0b000000000000000111111111111111100000000000000001111111111111111;
    //                                                  x == 0000 0000 0000 0000 0000 Z??? ???? ???? 0000 0000 0000 0000 ???? ???? ???? ???A
    x = (x ^ (x << 8)) & 0b000000000001111000000001111111100000000111111110000000011111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 ???? ???? 0000 0000 ???? ???? 0000 0000 ???? ???A
    x = (x ^ (x << 4)) & 0b000000000001111000011110000111100001111000011110000111100001111;
    //                                                  x == 0000 0000 0000 Z??? 0000 ???? 0000 ???? 0000 ???? 0000 ???? 0000 ???? 0000 ???A
    x = (x ^ (x << 2)) & 0b000000000110011001100110011001100110011001100110011001100110011;
    //                                                  x == 0000 0000 00Z? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?A
    x = (x ^ (x << 1)) & 0b000000001010101010101010101010101010101010101010101010101010101;
    //                                                  x == 0000 0000 0Z0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0A
    // now has it's original least 28 bits interleaved
    return x;
}
int64_t ZQuadTree::mask_and_contract(int64_t x){
    x = abs(x);
    x &= 0b000000001010101010101010101010101010101010101010101010101010101;
    //                                                  x == 0000 0000 0Z0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0? 0?0A
    x = (x ^ (x >> 1)) & 0b000000000110011001100110011001100110011001100110011001100110011;
    //                                                  x == 0000 0000 00Z? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?? 00?A
    x = (x ^ (x >> 2)) & 0b000000000001111000011110000111100001111000011110000111100001111;
    //                                                  x == 0000 0000 0000 Z??? 0000 ???? 0000 ???? 0000 ???? 0000 ???? 0000 ???? 0000 ???A
    x = (x ^ (x >> 4)) & 0b000000000001111000000001111111100000000111111110000000011111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 ???? ???? 0000 0000 ???? ???? 0000 0000 ???? ???A
    x = (x ^ (x >> 8)) & 0b000000000000000111111111111111100000000000000001111111111111111;
    //                                                  x == 0000 0000 0000 0000 0000 Z??? ???? ???? 0000 0000 0000 0000 ???? ???? ???? ???A
    x = (x ^ (x >> 16)) & 0b000000000000000000000000000000000001111111111111111111111111111;
    //                                                  x == 0000 0000 0000 0000 0000 0000 0000 0000 0000 Z??? ???? ???? ???? ???? ???? ???A
    return x;
}

int64_t ZQuadTree::morton_encode_xy(int64_t x, int64_t y){
    return cut_and_dilate(x) + (cut_and_dilate(y) << 1);
}
int64_t ZQuadTree::morton_decode_x(int64_t code){
    return mask_and_contract(code);
}
int64_t ZQuadTree::morton_decode_y(int64_t code){
    return mask_and_contract(code >> 1);
}

int64_t ZQuadTree::floating_to_integral_x(float x){
    float left = initial_quadrant_center.x - initial_quadrant_span;
    float right = initial_quadrant_center.x + initial_quadrant_span;
    return int((x-left)/(right - left));
}
int64_t ZQuadTree::floating_to_integral_y(float y){
    float bottom = initial_quadrant_center.y - initial_quadrant_span;
    float top = initial_quadrant_center.y + initial_quadrant_span;
    return int((y-bottom)/(top - bottom));
}

int64_t ZQuadTree::find_depth(){
    return total_depth;
}

TypedArray<ZLocationalCode> ZQuadTree::layout_leaves(){
    TypedArray<ZLocationalCode> arr;
    int64_t i = 0;
    int64_t size = pow(2, total_depth);
    while(i < cut_and_dilate(size)){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code(i);
        z_code->set_depth(total_depth);
        arr.push_back(z_code);
        i++;
    }
    return arr;
}

Ref<ZLocationalCode> ZQuadTree::leaf_that_contains_point(Vector2 point){
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_encode_xy(floating_to_integral_x(point.x), floating_to_integral_y(point.y)));
    z_code->set_depth(total_depth);
    return z_code;
}


Vector2 ZQuadTree::get_quadrant_center(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() == 0)
        return initial_quadrant_center;

    int64_t integral_x = morton_decode_x(quadrant->get_morton_code());
    int64_t integral_y = morton_decode_y(quadrant->get_morton_code());

    float quadrant_span = get_quadrant_span(quadrant->get_depth());

    Vector2 initial_quadrant_bsw = initial_quadrant_center - Vector2(initial_quadrant_span, initial_quadrant_span);

    return initial_quadrant_bsw + Vector2(integral_x * 2*quadrant_span + quadrant_span, (integral_y * 2*quadrant_span + quadrant_span));
}
float ZQuadTree::get_quadrant_span(int64_t depth){
    if(depth == 0)
        return initial_quadrant_span;
    // if(depth > total_depth)
    //  LOG_ERROR

    return initial_quadrant_span / pow(2, depth);
}

Ref<ZLocationalCode> ZQuadTree::get_quadrant_parent(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() == 0)
        return Ref<ZLocationalCode>();

    int64_t parent_depth = quadrant->get_depth()-1;
    int64_t size = pow(2, parent_depth);
    
    int64_t mask = cut_and_dilate(size)-1;

    int64_t new_code = quadrant->get_morton_code() & mask;

    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(new_code);
    z_code->set_depth(parent_depth);
    return z_code;
}
TypedArray<ZLocationalCode> ZQuadTree::get_quadrant_children(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() == total_depth)
        return TypedArray<ZLocationalCode>();
    TypedArray<ZLocationalCode> arr;

    int64_t current_code = quadrant->get_morton_code();
    int64_t child_depth = quadrant->get_depth()+1;
    
    for(int64_t i = 0; i < 4; i++){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code(current_code+i);
        z_code->set_depth(child_depth);
        arr.push_back(z_code);
    }
    return arr;
}


Ref<ZLocationalCode> ZQuadTree::find_neighbor_of_equal_depth(Ref<ZLocationalCode> quadrant, int64_t neighbor){
    int64_t current_depth = quadrant->get_depth();
    if(current_depth==0)
        return Ref<ZLocationalCode>();
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_depth(current_depth);
    switch (neighbor){
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_WEST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(), current_depth, -1), current_depth, -1));
        case QUADRANT_NEIGHBOR::QUADRANT_WEST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(quadrant->get_morton_code(), current_depth, -1));
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_WEST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1),current_depth, -1));
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH:
            z_code->set_morton_code(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1));
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH:
            z_code->set_morton_code(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, -1));
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_EAST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, -1),current_depth, 1));
        case QUADRANT_NEIGHBOR::QUADRANT_EAST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1));
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_EAST:
            z_code->set_morton_code(morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1),current_depth, 1));
        default:
            z_code->set_morton_code(-1);
    }
    return z_code;
}
TypedArray<ZLocationalCode> ZQuadTree::find_neighbors_of_equal_depth(Ref<ZLocationalCode> quadrant, PackedInt64Array neighbors){
    TypedArray<ZLocationalCode> arr;
    arr.resize(neighbors.size());
    for(int64_t i = 0; i < neighbors.size(); i++){
        arr[i] = find_neighbor_of_equal_depth(quadrant, (QUADRANT_NEIGHBOR) ((int64_t) neighbors[i]));
    }
    return arr;
}
TypedArray<ZLocationalCode> ZQuadTree::find_leaf_neighbors(Ref<ZLocationalCode> quadrant, int64_t neighbor){
    return TypedArray<ZLocationalCode>();
}

int64_t ZQuadTree::morton_add_x_and_null_out_of_domain(int64_t code, int64_t depth, int64_t term){
    if(code < 0)
        return -1;
    if(term == 0)
        return code;
    if(term < 0){
        int64_t sub = ((code & 0b000000001010101010101010101010101010101010101010101010101010101) - term) & 0b000000001010101010101010101010101010101010101010101010101010101;
        int64_t res = ((code & 0b000000010101010101010101010101010101010101010101010101010101010) | sub);
        if(res < 0)
            return -1;
        else
            return res;
    }
    else{
        int64_t res = (code | (((code | 0b000000010101010101010101010101010101010101010101010101010101010) + term) & 0b000000001010101010101010101010101010101010101010101010101010101));
        if(res > pow(2, depth))
            return -1;
        else
            return res;
    }
}

int64_t ZQuadTree::morton_add_y_and_null_out_of_domain(int64_t code, int64_t depth, int64_t term){
    if(code < 0)
        return -1;
    if(term == 0)
        return code;
    if(term < 0){
        int64_t sub = ((code & 0b000000010101010101010101010101010101010101010101010101010101010) - term) & 0b000000010101010101010101010101010101010101010101010101010101010;
        int64_t res = ((code & 0b000000001010101010101010101010101010101010101010101010101010101) | sub);
        if(res < 0)
            return -1;
        else
            return res;
    }else{
        int64_t res = (code | (((code | 0b000000001010101010101010101010101010101010101010101010101010101) + term) & 0b000000010101010101010101010101010101010101010101010101010101010));
        if(res > pow(2, depth))
            return -1;
        else
            return res;
    }
}

void ZQuadTree::split(int64_t _total_depth){
    total_depth = _total_depth;
}
void ZQuadTree::readjust(Vector2 _initial_quadrant_center, float _initial_quadrant_span){
    initial_quadrant_center = _initial_quadrant_center;
    initial_quadrant_span = _initial_quadrant_span;
}

PackedVector2Array ZQuadTree::gen_lines(int64_t at_depth){

    PackedVector2Array line_vertices;

    Vector2 initial_quadrant_bsw = initial_quadrant_center - Vector2(initial_quadrant_span, initial_quadrant_span);

    int64_t size = pow(2, at_depth);

    float span = get_quadrant_span(at_depth);

    for(int64_t j = 0; j <= size; j++){
        Vector2 line_begin = initial_quadrant_bsw +  Vector2(2*span*j, 0);
        Vector2 line_end = initial_quadrant_bsw + Vector2(2*span*j, 2*initial_quadrant_span);
        line_vertices.push_back(line_begin);
        line_vertices.push_back(line_end);
    }

    for(int64_t j = 0; j <= size; j++){
        Vector2 line_begin = initial_quadrant_bsw +  Vector2(0, 2*span*j);
        Vector2 line_end = initial_quadrant_bsw + Vector2(2*initial_quadrant_span, 2*span*j);
        line_vertices.push_back(line_begin);
        line_vertices.push_back(line_end);
    }


    return line_vertices;

}
PackedVector2Array ZQuadTree::gen_z_lines(int64_t at_depth){
    PackedVector2Array line_vertices;

    int64_t size = pow(2, at_depth);
    int64_t i = 0;
    while(i < cut_and_dilate(size)-1){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code(i);
        z_code->set_depth(at_depth);
        line_vertices.push_back(get_quadrant_center(z_code));
        z_code.instantiate();
        z_code->set_morton_code(i+1);
        z_code->set_depth(at_depth);
        line_vertices.push_back(get_quadrant_center(z_code));
        i++;
    }
    

    return line_vertices;
}