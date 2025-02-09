#include "z_locational_code.h"

using namespace godot;

void ZLocationalCode::_bind_methods(){
    ClassDB::bind_static_method("ZLocationalCode", D_METHOD("create", "_morton_code", "_depth"), &ZLocationalCode::create);
    ClassDB::bind_method(D_METHOD("get_morton_code"), &ZLocationalCode::get_morton_code);
    ClassDB::bind_method(D_METHOD("set_morton_code", "p_code"), &ZLocationalCode::set_morton_code);
    ClassDB::bind_method(D_METHOD("get_depth"), &ZLocationalCode::get_depth);
    ClassDB::bind_method(D_METHOD("set_depth", "p_depth"), &ZLocationalCode::set_depth);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "morton_code"), "set_morton_code", "get_morton_code");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "depth"), "set_depth", "get_depth");


}


ZLocationalCode::ZLocationalCode(int64_t _morton_code, int64_t _depth){
    morton_code = _morton_code;
    depth = _depth;
}
ZLocationalCode::ZLocationalCode(){
    morton_code = 0;
    depth = 0;
}
ZLocationalCode::~ZLocationalCode(){

}


Ref<ZLocationalCode> ZLocationalCode::create(int64_t _morton_code, int64_t _depth){
    Ref<ZLocationalCode> res;
    res.instantiate();
    res->morton_code = _morton_code;
    res->depth = _depth;
    return res;
}


int64_t ZLocationalCode::get_morton_code(){
    return morton_code;
}
void ZLocationalCode::set_morton_code(int64_t p_code){
    morton_code = p_code;
}
int64_t ZLocationalCode::get_depth(){
    return depth;
}
void ZLocationalCode::set_depth(int64_t p_depth){
    depth = p_depth;
}


