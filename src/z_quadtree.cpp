#include "z_quadtree.h"

using namespace godot;

void ZQuadTree::_bind_methods(){

    ClassDB::bind_static_method("ZQuadTree", D_METHOD("create", "total_depth", "initial_quadrant_center", "initial_quadrant_span"), &ZQuadTree::create);   
    ClassDB::bind_method(D_METHOD("get_bit_upper_bound"), &ZQuadTree::get_bit_upper_bound);
    ClassDB::bind_method(D_METHOD("find_depth"), &ZQuadTree::find_depth);
    ClassDB::bind_method(D_METHOD("layout_leaves"), &ZQuadTree::layout_leaves);
	ClassDB::bind_method(D_METHOD("leaf_that_contains_point", "point"), &ZQuadTree::leaf_that_contains_point);
	ClassDB::bind_method(D_METHOD("cell_that_contains_point", "point", "at_depth"), &ZQuadTree::cell_that_contains_point);
    ClassDB::bind_method(D_METHOD("get_cell_center", "quadrant"), &ZQuadTree::get_cell_center);
    ClassDB::bind_method(D_METHOD("get_cell_span", "depth"), &ZQuadTree::get_cell_span);
    ClassDB::bind_method(D_METHOD("get_cell_parent", "quadrant"), &ZQuadTree::get_cell_parent);
    ClassDB::bind_method(D_METHOD("get_cell_children", "quadrant"), &ZQuadTree::get_cell_children);
    ClassDB::bind_method(D_METHOD("find_neighbor_of_equal_depth", "quadrant", "neighbor"), &ZQuadTree::find_neighbor_of_equal_depth);
	ClassDB::bind_method(D_METHOD("find_neighbors_of_equal_depth", "quadrant", "neighbors"), &ZQuadTree::find_neighbors_of_equal_depth);
    ClassDB::bind_method(D_METHOD("split", "total_depth"), &ZQuadTree::split);
	ClassDB::bind_method(D_METHOD("readjust", "initial_quadrant_center", "initial_quadrant_span"), &ZQuadTree::readjust);
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

Ref<ZQuadTree> ZQuadTree::create(int64_t _total_depth, Vector2 _initial_quadrant_center, real_t _initial_quadrant_span){
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

int64_t ZQuadTree::floating_to_integral_x(real_t x, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::floating_to_integral_x requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return 0;
    }
    real_t left = initial_quadrant_center.x - initial_quadrant_span;
    real_t right = initial_quadrant_center.x + initial_quadrant_span;
    return float((x-left)/(right - left) * (1LL << at_depth));
}
int64_t ZQuadTree::floating_to_integral_y(real_t y, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::floating_to_integral_y requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return 0;
    }
    real_t bottom = initial_quadrant_center.y - initial_quadrant_span;
    real_t top = initial_quadrant_center.y + initial_quadrant_span;
    return float((y-bottom)/(top - bottom) * (1LL << at_depth));
}

int64_t ZQuadTree::find_depth(){
    return total_depth;
}

TypedArray<ZLocationalCode> ZQuadTree::layout_leaves(){
    TypedArray<ZLocationalCode> arr;
    int64_t i = 0;
    int64_t size = (1LL << total_depth);
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
    if(!(point.x >= initial_quadrant_center.x-initial_quadrant_span && point.x <= initial_quadrant_center.x + initial_quadrant_span
        && point.y >= initial_quadrant_center.y - initial_quadrant_span && point.y <= initial_quadrant_center.y + initial_quadrant_span)){
            return Ref<ZLocationalCode>();
     }
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_encode_xy(floating_to_integral_x(point.x, total_depth), floating_to_integral_y(point.y, total_depth)));
    z_code->set_depth(total_depth);
    return z_code;
}

Ref<ZLocationalCode> ZQuadTree::cell_that_contains_point(Vector2 point, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::cell_that_contains_point requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    if(!(point.x >= initial_quadrant_center.x-initial_quadrant_span && point.x <= initial_quadrant_center.x + initial_quadrant_span
        && point.y >= initial_quadrant_center.y - initial_quadrant_span && point.y <= initial_quadrant_center.y + initial_quadrant_span)){
            return Ref<ZLocationalCode>();
    }
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_encode_xy(floating_to_integral_x(point.x, at_depth), floating_to_integral_y(point.y, at_depth)));
    z_code->set_depth(at_depth);
    return z_code;
}


Vector2 ZQuadTree::get_cell_center(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() > total_depth || quadrant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::get_quadrant_center requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return Vector2();
    }
    if(quadrant->get_depth() == 0)
        return initial_quadrant_center;

    int64_t integral_x = morton_decode_x(quadrant->get_morton_code());
    int64_t integral_y = morton_decode_y(quadrant->get_morton_code());

    real_t quadrant_span = get_cell_span(quadrant->get_depth());

    Vector2 initial_quadrant_bsw = initial_quadrant_center - Vector2(initial_quadrant_span, initial_quadrant_span);

    return initial_quadrant_bsw + Vector2(integral_x * 2*quadrant_span + quadrant_span, (integral_y * 2*quadrant_span + quadrant_span));
}
real_t ZQuadTree::get_cell_span(int64_t depth){
    if(depth == 0)
        return initial_quadrant_span;
    // if(depth > total_depth)
    //  LOG_ERROR

    return initial_quadrant_span / (1LL << depth);
}

Ref<ZLocationalCode> ZQuadTree::get_cell_parent(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() > total_depth || quadrant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::get_quadrant_parent requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    if(quadrant->get_depth() == 0)
        return Ref<ZLocationalCode>();

    int64_t parent_depth = quadrant->get_depth()-1;

    int64_t new_code = quadrant->get_morton_code() >> 2;

    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(new_code);
    z_code->set_depth(parent_depth);
    return z_code;
}
TypedArray<ZLocationalCode> ZQuadTree::get_cell_children(Ref<ZLocationalCode> quadrant){
    if(quadrant->get_depth() > total_depth || quadrant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::get_quadrant_children requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return TypedArray<ZLocationalCode>();
    }
    if(quadrant->get_depth() == total_depth)
        return TypedArray<ZLocationalCode>();
    TypedArray<ZLocationalCode> arr;

    int64_t current_code = quadrant->get_morton_code();
    int64_t child_depth = quadrant->get_depth()+1;
    
    for(int64_t i = 0; i < 4; i++){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code((current_code << 2) | i);
        z_code->set_depth(child_depth);
        arr.push_back(z_code);
    }
    return arr;
}


Ref<ZLocationalCode> ZQuadTree::find_neighbor_of_equal_depth(Ref<ZLocationalCode> quadrant, int64_t neighbor){
    int64_t current_depth = quadrant->get_depth();
    if(current_depth > total_depth || current_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::gen_lines requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    if(current_depth==0)
        return Ref<ZLocationalCode>();
    int64_t morton_code;
    switch (neighbor){
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(), current_depth, -1), current_depth, -1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(quadrant->get_morton_code(), current_depth, -1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1),current_depth, -1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH:
            morton_code = (morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH:
            morton_code = (morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, -1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_SOUTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, -1),current_depth, 1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1));
            break;
        case QUADRANT_NEIGHBOR::QUADRANT_NORTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(quadrant->get_morton_code(),current_depth, 1),current_depth, 1));
            break;
        default:
            morton_code = (-1);
            break;
    }
    if(morton_code == -1)
        return Ref<ZLocationalCode>();
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_code);
    z_code->set_depth(current_depth);
    return z_code;
}
TypedArray<ZLocationalCode> ZQuadTree::find_neighbors_of_equal_depth(Ref<ZLocationalCode> quadrant, PackedInt64Array neighbors){
    if(quadrant->get_depth() > total_depth || quadrant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::gen_lines requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return TypedArray<ZLocationalCode>();
    }
    TypedArray<ZLocationalCode> arr;
    arr.resize(neighbors.size());
    for(int64_t i = 0; i < neighbors.size(); i++){
        arr[i] = find_neighbor_of_equal_depth(quadrant, (QUADRANT_NEIGHBOR) ((int64_t) neighbors[i]));
    }
    return arr;
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
        if(res > (1LL << depth))
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
        if(res > (1LL << depth))
            return -1;
        else
            return res;
    }
}

void ZQuadTree::split(int64_t _total_depth){
    if(_total_depth > ZQUADTREE_UPPERBOUND){
        ERR_PRINT_ONCE("Method ZQuadTree::split requires a total depth less than or equal 27");
        return;
    }
    total_depth = _total_depth;
}
void ZQuadTree::readjust(Vector2 _initial_quadrant_center, real_t _initial_quadrant_span){
    initial_quadrant_center = _initial_quadrant_center;
    initial_quadrant_span = _initial_quadrant_span;
}

PackedVector2Array ZQuadTree::gen_lines(int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::gen_lines requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return PackedVector2Array();
    }
    PackedVector2Array line_vertices;

    Vector2 initial_quadrant_bsw = initial_quadrant_center - Vector2(initial_quadrant_span, initial_quadrant_span);

    int64_t size = (1LL << at_depth);

    real_t span = get_cell_span(at_depth);

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
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZQuadTree::gen_z_lines requires a depth in the bounds of 0 and ZQuadTree::find_depth()");
        return PackedVector2Array();
    }
    PackedVector2Array line_vertices;

    int64_t size = (1LL << at_depth);
    int64_t i = 0;
    while(i < cut_and_dilate(size)-1){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code(i);
        z_code->set_depth(at_depth);
        line_vertices.push_back(get_cell_center(z_code));
        z_code.instantiate();
        z_code->set_morton_code(i+1);
        z_code->set_depth(at_depth);
        line_vertices.push_back(get_cell_center(z_code));
        i++;
    }
    

    return line_vertices;
}