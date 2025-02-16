#include "z_octree.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/mesh.hpp>


using namespace godot;

void ZOcTree::_bind_methods(){
    ClassDB::bind_static_method("ZOcTree", D_METHOD("create", "total_depth", "initial_octant_center", "initial_octant_span"), &ZOcTree::create);   
    ClassDB::bind_method(D_METHOD("get_bit_upper_bound"), &ZOcTree::get_bit_upper_bound);
    ClassDB::bind_method(D_METHOD("find_depth"), &ZOcTree::find_depth);
    ClassDB::bind_method(D_METHOD("layout_leaves"), &ZOcTree::layout_leaves);
	ClassDB::bind_method(D_METHOD("leaf_that_contains_point", "point"), &ZOcTree::leaf_that_contains_point);
	ClassDB::bind_method(D_METHOD("cell_that_contains_point", "point", "at_depth"), &ZOcTree::cell_that_contains_point);
    ClassDB::bind_method(D_METHOD("get_cell_center", "octant"), &ZOcTree::get_cell_center);
    ClassDB::bind_method(D_METHOD("get_cell_span", "depth"), &ZOcTree::get_cell_span);
    ClassDB::bind_method(D_METHOD("get_cell_parent", "octant"), &ZOcTree::get_cell_parent);
    ClassDB::bind_method(D_METHOD("get_cell_children", "octant"), &ZOcTree::get_cell_children);
    ClassDB::bind_method(D_METHOD("find_neighbor_of_equal_depth", "octant", "neighbor"), &ZOcTree::find_neighbor_of_equal_depth);
	ClassDB::bind_method(D_METHOD("find_neighbors_of_equal_depth", "octant", "neighbors"), &ZOcTree::find_neighbors_of_equal_depth);
    ClassDB::bind_method(D_METHOD("split", "total_depth"), &ZOcTree::split);
	ClassDB::bind_method(D_METHOD("readjust", "initial_octant_center", "initial_octant_span"), &ZOcTree::readjust);
	ClassDB::bind_method(D_METHOD("gen_arrays", "at_depth"), &ZOcTree::gen_arrays);
	ClassDB::bind_method(D_METHOD("gen_z_arrays", "at_depth"), &ZOcTree::gen_z_arrays);


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
    

}

ZOcTree::ZOcTree(){

}
ZOcTree::~ZOcTree(){

}

Ref<ZOcTree> ZOcTree::create(int64_t _total_depth, Vector3 _initial_octant_center, real_t _initial_octant_span){
    Ref<ZOcTree> res;
    res.instantiate();
    res->total_depth = _total_depth;
    res->initial_octant_center = _initial_octant_center;
    res->initial_octant_span = _initial_octant_span;
    return res;
}

int64_t ZOcTree::cut_and_dilate(int64_t x){
    x = abs(x);
    // x is at this point 63 bits of whatever:          x == 0??? ???? ???? ???? ???? ???? ???? ???? ???? ???? ???? Z??? ???? ???? ???? ???A
    x &= 0b000000000000000000000000000000000000000000011111111111111111111;
    // now 44 bits of nothing and 20 bits of whatever:  x == 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 Z??? ???? ???? ???? ???A
    x = (x ^ (x << 32)) & 0b000000000001111000000000000000000000000000000001111111111111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 0000 0000 0000 0000 0000 0000 ???? ???? ???? ???A
    x = (x ^ (x << 16)) & 0b000000000001111000000000000000011111111000000000000000011111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 0000 0000 ???? ???? 0000 0000 0000 0000 ???? ???A
    x = (x ^ (x << 8)) & 0b000000000001111000000001111000000001111000000001111000000001111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 ???? 0000 0000 ???? 0000 0000 ???? 0000 0000 ???A
    x = (x ^ (x << 4)) & 0b000000011000011000011000011000011000011000011000011000011000011;
    //                                                  x == 0000 0000 Z?00 00?? 0000 ??00 00?? 0000 ??00 00?? 0000 ??00 00?? 0000 ??00 00?A
    x = (x ^ (x << 2)) & 0b000001001001001001001001001001001001001001001001001001001001001;
    //                                                  x == 0000 00Z0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00A
    // now has it's original least 20 bits interleaved
    return x;
}
int64_t ZOcTree::mask_and_contract(int64_t x){
    x = abs(x);
    x &= 0b000001001001001001001001001001001001001001001001001001001001001;;
    //                                                  x == 0000 00Z0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00? 00?0 0?00 ?00A
    x = (x ^ (x >> 2)) & 0b000000011000011000011000011000011000011000011000011000011000011;
    //                                                  x == 0000 0000 Z?00 00?? 0000 ??00 00?? 0000 ??00 00?? 0000 ??00 00?? 0000 ??00 00?A
    x = (x ^ (x >> 4)) & 0b000000000001111000000001111000000001111000000001111000000001111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 ???? 0000 0000 ???? 0000 0000 ???? 0000 0000 ???A
    x = (x ^ (x >> 8)) & 0b000000000001111000000000000000011111111000000000000000011111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 0000 0000 ???? ???? 0000 0000 0000 0000 ???? ???A
    x = (x ^ (x >> 16)) & 0b000000000001111000000000000000000000000000000001111111111111111;
    //                                                  x == 0000 0000 0000 Z??? 0000 0000 0000 0000 0000 0000 0000 0000 ???? ???? ???? ???A
    x = (x ^ (x >> 32)) & 0b000000000000000000000000000000000000000000011111111111111111111;
    //                                                  x == 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 Z??? ???? ???? ???? ???A
    return x;
}

int64_t ZOcTree::morton_encode_xyz(int64_t x, int64_t y, int64_t z){
    return cut_and_dilate(x) + (cut_and_dilate(y) << 1) + (cut_and_dilate(z) << 2);
}
int64_t ZOcTree::morton_decode_x(int64_t code){
    return mask_and_contract(code);
}
int64_t ZOcTree::morton_decode_y(int64_t code){
    return mask_and_contract(code >> 1);
}
int64_t ZOcTree::morton_decode_z(int64_t code){
    return mask_and_contract(code >> 2);
}

int64_t ZOcTree::floating_to_integral_x(real_t x, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::floating_to_integral_x requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return 0;
    }
    real_t left = initial_octant_center.x - initial_octant_span;
    real_t right = initial_octant_center.x + initial_octant_span;
    return floor((x-left)/(right - left) * (1LL << at_depth));
}
int64_t ZOcTree::floating_to_integral_y(real_t y, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::floating_to_integral_y requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return 0;
    }
    real_t bottom = initial_octant_center.y - initial_octant_span;
    real_t top = initial_octant_center.y + initial_octant_span;
    return floor((y-bottom)/(top - bottom) * (1LL << at_depth));
}
int64_t ZOcTree::floating_to_integral_z(real_t z, int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::floating_to_integral_z requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return 0;
    }
    real_t back = initial_octant_center.z - initial_octant_span;
    real_t forward = initial_octant_center.z + initial_octant_span;
    return floor((z-back)/(forward - back) * (1LL << at_depth));
}

int64_t ZOcTree::find_depth(){
    return total_depth;
}
TypedArray<ZLocationalCode> ZOcTree::layout_leaves(){
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
Ref<ZLocationalCode> ZOcTree::leaf_that_contains_point(Vector3 point){
    if(!(point.x >= initial_octant_center.x-initial_octant_span && point.x <= initial_octant_center.x + initial_octant_span
        && point.y >= initial_octant_center.y - initial_octant_span && point.y <= initial_octant_center.y + initial_octant_span
        && point.z >= initial_octant_center.z - initial_octant_span && point.z <= initial_octant_center.z + initial_octant_span)){
        return Ref<ZLocationalCode>();
    }
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_encode_xyz(floating_to_integral_x(point.x, total_depth), floating_to_integral_y(point.y, total_depth), floating_to_integral_z(point.z, total_depth)));
    z_code->set_depth(total_depth);
    return z_code;
}

Ref<ZLocationalCode> ZOcTree::cell_that_contains_point(Vector3 point, int64_t at_depth){
    if(!(point.x >= initial_octant_center.x-initial_octant_span && point.x <= initial_octant_center.x + initial_octant_span
        && point.y >= initial_octant_center.y - initial_octant_span && point.y <= initial_octant_center.y + initial_octant_span
        && point.z >= initial_octant_center.z - initial_octant_span && point.z <= initial_octant_center.z + initial_octant_span)){
        return Ref<ZLocationalCode>();
    }
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::cell_that_contains_point requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(morton_encode_xyz(floating_to_integral_x(point.x, at_depth), floating_to_integral_y(point.y, at_depth), floating_to_integral_z(point.z, at_depth)));
    z_code->set_depth(at_depth);
    return z_code;
}

Vector3 ZOcTree::get_cell_center(Ref<ZLocationalCode> octant){
    if(octant->get_depth() > total_depth || octant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZOcTree::get_octant_center requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return Vector3();
    }
    int64_t integral_x = morton_decode_x(octant->get_morton_code());
    int64_t integral_y = morton_decode_y(octant->get_morton_code());
    int64_t integral_z = morton_decode_z(octant->get_morton_code());

    real_t octant_span = get_cell_span(octant->get_depth());

    Vector3 initial_octant_bsw = initial_octant_center - Vector3(initial_octant_span, initial_octant_span, initial_octant_span);

    return initial_octant_bsw + Vector3(integral_x * 2*octant_span + octant_span, integral_y * 2*octant_span + octant_span, integral_z * 2*octant_span + octant_span);
}
real_t ZOcTree::get_cell_span(int64_t depth){
    return initial_octant_span / (1LL << depth);
}

Ref<ZLocationalCode> ZOcTree::get_cell_parent(Ref<ZLocationalCode> octant){
    if(octant->get_depth() > total_depth || octant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZOcTree::get_octant_parent requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    if(octant->get_depth() == 0)
        return Ref<ZLocationalCode>();

    int64_t parent_depth = octant->get_depth()-1;
    int64_t new_code = octant->get_morton_code() >> 3;

    Ref<ZLocationalCode> z_code;
    z_code.instantiate();
    z_code->set_morton_code(new_code);
    z_code->set_depth(parent_depth);
    return z_code;
}
TypedArray<ZLocationalCode> ZOcTree::get_cell_children(Ref<ZLocationalCode> octant){
    if(octant->get_depth() > total_depth || octant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZOcTree::get_octant_children requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return TypedArray<ZLocationalCode>();
    }
    if(octant->get_depth() == total_depth)
        return TypedArray<ZLocationalCode>();
    
    TypedArray<ZLocationalCode> arr;

    int64_t current_code = octant->get_morton_code();
    int64_t child_depth = octant->get_depth()+1;
    
    for(int64_t i = 0; i < 8; i++){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code((current_code << 3) | i);
        z_code->set_depth(child_depth);
        arr.push_back(z_code);
    }
    return arr;
}


Ref<ZLocationalCode> ZOcTree::find_neighbor_of_equal_depth(Ref<ZLocationalCode> octant, int64_t neighbor){
    int64_t current_depth = octant->get_depth();
    if(current_depth > total_depth || current_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::find_neighbor_of_equal_depth requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return Ref<ZLocationalCode>();
    }
    if(current_depth==0)
        return Ref<ZLocationalCode>();
    
    int64_t morton_code;
    switch (neighbor){
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, -1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, -1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH:
            morton_code = (morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW:
            morton_code = (morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH:
            morton_code = (morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_SOUTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, -1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_BELOW_NORTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, -1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_SOUTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_NORTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_NORTH:
            morton_code = (morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_SOUTH:
            morton_code = (morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_SOUTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_NORTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, 1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_WEST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH:
            morton_code = (morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE:
            morton_code = (morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH:
            morton_code = (morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_SOUTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, -1), current_depth, 1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1));
            break;
        case OCTANT_NEIGHBOR::OCTANT_ABOVE_NORTH_EAST:
            morton_code = (morton_add_x_and_null_out_of_domain(morton_add_y_and_null_out_of_domain(morton_add_z_and_null_out_of_domain(octant->get_morton_code(), current_depth, 1), current_depth, 1), current_depth, 1));
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
TypedArray<ZLocationalCode> ZOcTree::find_neighbors_of_equal_depth(Ref<ZLocationalCode> octant, PackedInt64Array neighbors){
    if(octant->get_depth() > total_depth || octant->get_depth() < 0){
        ERR_PRINT_ONCE("Method ZOcTree::find_neighbors_of_equal_depth requires an octant with depth in the bounds of 0 and ZOcTree::find_depth()");
        return TypedArray<ZLocationalCode>();
    }
    TypedArray<ZLocationalCode> arr;
    arr.resize(neighbors.size());
    for(int64_t i = 0; i < neighbors.size(); i++){
        arr[i] = find_neighbor_of_equal_depth(octant, (OCTANT_NEIGHBOR) ((int64_t) neighbors[i]));
    }
    return arr;
}

int64_t ZOcTree::morton_add_x_and_null_out_of_domain(int64_t code, int64_t depth, int64_t term){
    if(code < 0)
        return -1;
    if(term == 0)
        return code;
    if(term < 0){
        int64_t sub = ((code & 0b000001001001001001001001001001001001001001001001001001001001001) - term) & 0b000001001001001001001001001001001001001001001001001001001001001;
        int64_t res = ((code & 0b000110110110110110110110110110110110110110110110110110110110110) | sub);
        if(res < 0)
            return -1;
        else
            return res;
    }
    else{
        int64_t res = (code | (((code | 0b000110110110110110110110110110110110110110110110110110110110110) + term) & 0b000001001001001001001001001001001001001001001001001001001001001));
        if(res > (1LL << depth))
            return -1;
        else
            return res;
    }
}
int64_t ZOcTree::morton_add_y_and_null_out_of_domain(int64_t code, int64_t depth, int64_t term){
    if(code < 0)
        return -1;
    if(term == 0)
        return code;
    if(term < 0){
        int64_t sub = ((code & 0b000010010010010010010010010010010010010010010010010010010010010) - term) & 0b000010010010010010010010010010010010010010010010010010010010010;
        int64_t res = ((code & 0b000101101101101101101101101101101101101101101101101101101101101) | sub);
        if(res < 0)
            return -1;
        else
            return res;
    }
    else{
        int64_t res = (code | (((code | 0b000101101101101101101101101101101101101101101101101101101101101) + term) & 0b000010010010010010010010010010010010010010010010010010010010010));
        if(res > (1LL << depth))
            return -1;
        else
            return res;
    }
}
int64_t ZOcTree::morton_add_z_and_null_out_of_domain(int64_t code, int64_t depth, int64_t term){
    if(code < 0)
        return -1;
    if(term == 0)
        return code;
    if(term < 0){
        int64_t sub = ((code & 0b000100100100100100100100100100100100100100100100100100100100100) - term) & 0b000100100100100100100100100100100100100100100100100100100100100;
        int64_t res = ((code & 0b000011011011011011011011011011011011011011011011011011011011011) | sub);
        if(res < 0)
            return -1;
        else
            return res;
    }
    else{
        int64_t res = (code | (((code | 0b000011011011011011011011011011011011011011011011011011011011011) + term) & 0b000100100100100100100100100100100100100100100100100100100100100));
        if(res > (1LL << depth))
            return -1;
        else
            return res;
    }
}

void ZOcTree::split(int64_t _total_depth){
    if(_total_depth > ZOCTREE_UPPERBOUND){
        ERR_PRINT_ONCE("Method ZOcTree::split requires a total depth less than or equal to 19");
        return;
    }

    total_depth = _total_depth;
}
void ZOcTree::readjust(Vector3 _initial_octant_center, real_t _initial_octant_span){
    initial_octant_center = _initial_octant_center;
    initial_octant_span = _initial_octant_span;
}

Array ZOcTree::gen_arrays(int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::gen_arrays requires a depth in the bounds of 0 and ZOcTree::find_depth()");
        return Array();
    }

    PackedVector3Array vertices;

    Vector3 initial_octant_bsw = initial_octant_center - Vector3(initial_octant_span, initial_octant_span, initial_octant_span);

    int64_t size = (1LL << at_depth);

    real_t span = get_cell_span(at_depth);

    //Uniform grid construction

    //HORIZONTAL
    for(int64_t i = 0; i <= size; i++){
        for(int64_t j = 0; j <= size; j++){
            Vector3 line_begin = initial_octant_bsw + Vector3(0, 2*span*i, 0) + Vector3(2*span*j, 0, 0);
            Vector3 line_end = initial_octant_bsw + Vector3(0, 2*span*i, 0) + Vector3(2*span*j, 0, 2*initial_octant_span);
            vertices.push_back(line_begin);
            vertices.push_back(line_end);
        }
    }

    //DEEP
    for(int64_t i = 0; i <= size; i++){
        for(int64_t j = 0; j <= size; j++){
            Vector3 line_begin = initial_octant_bsw + Vector3(0, 2*span*i, 0) + Vector3(0, 0, 2*span*j);
            Vector3 line_end = initial_octant_bsw + Vector3(0, 2*span*i, 0) + Vector3(2*initial_octant_span, 0, 2*span*j);
            vertices.push_back(line_begin);
            vertices.push_back(line_end);
        }
    }

    //VERTICAL
    for(int64_t i = 0; i <= size; i++){
        for(int64_t j = 0; j <= size; j++){
            Vector3 line_begin = initial_octant_bsw + Vector3(2*span*i, 0, 2*span*j);
            Vector3 line_end = initial_octant_bsw + Vector3(0, 2*initial_octant_span, 0) + Vector3(2*span*i, 0, 2*span*j);
            vertices.push_back(line_begin);
            vertices.push_back(line_end);
        }
    }


    Array arr;
    arr.resize(Mesh::ARRAY_MAX);
    arr[Mesh::ARRAY_VERTEX] = vertices;
    return arr;
}
Array ZOcTree::gen_z_arrays(int64_t at_depth){
    if(at_depth > total_depth || at_depth < 0){
        ERR_PRINT_ONCE("Method ZOcTree::gen_z_arrays requires a depth in the bounds of 0 and ZOcTree::find_depth()");
        return Array();
    }
    PackedVector3Array vertices;
    PackedInt32Array indices;

    int64_t size = (1LL << at_depth);
    int64_t i = 0;
    while(i < cut_and_dilate(size)){
        Ref<ZLocationalCode> z_code;
        z_code.instantiate();
        z_code->set_morton_code(i);
        z_code->set_depth(at_depth);
        vertices.push_back(get_cell_center(z_code));
        i++;
    }

    for(int64_t i = 0; i < vertices.size()-1; i++){
        indices.push_back(i);
        indices.push_back(i+1);
    }

    Array arr;
    arr.resize(Mesh::ARRAY_MAX);
    arr[Mesh::ARRAY_VERTEX] = vertices;
    arr[Mesh::ARRAY_INDEX] = indices;
    return arr;
}