#include "flat_octree_octant.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FlatOcTreeOctant::_bind_methods(){
    ClassDB::bind_method(D_METHOD("get_center"), &FlatOcTreeOctant::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &FlatOcTreeOctant::set_center);
    ClassDB::bind_method(D_METHOD("get_span"), &FlatOcTreeOctant::get_span);
	ClassDB::bind_method(D_METHOD("set_span", "span"), &FlatOcTreeOctant::set_span);
	ClassDB::bind_method(D_METHOD("get_parent"), &FlatOcTreeOctant::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "parent"), &FlatOcTreeOctant::set_parent);
	ClassDB::bind_method(D_METHOD("get_children"), &FlatOcTreeOctant::get_children);
	ClassDB::bind_method(D_METHOD("set_children", "children"), &FlatOcTreeOctant::set_children);
    
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "span"), "set_span", "get_span");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "parent"), "set_parent", "get_parent");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "children"), "set_children", "get_children");

    //VIRTUAL METHODS
    GDVIRTUAL_BIND(_to_dict);
    GDVIRTUAL_BIND(_copy_from_dict, "dict");
    GDVIRTUAL_BIND(_unflatten);
    GDVIRTUAL_BIND(_destroy);
}

//VIRTUAL DEFINITIONS
GDVIRTUAL0R_DEFINE(OcTreeOctant, Ref<FlatOcTreeOctant>, _flatten);
//To be only called by OcTree::flatten()
Ref<FlatOcTreeOctant> OcTreeOctant::flatten(){
    Ref<FlatOcTreeOctant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_flatten)){
        _gdvirtual__flatten_call<false>(res);
    }
    if(res.is_null())
        res.instantiate();
    res->center = center;
    res->span = span;
    res->children.resize(8);
    if(is_root())
        res->parent = -1;
    if(is_leaf())
        res->children.fill(-1);
    return res;
}

FlatOcTreeOctant::FlatOcTreeOctant(){
}
FlatOcTreeOctant::~FlatOcTreeOctant(){
    destroy();
}
void FlatOcTreeOctant::destroy(){
    if(GDVIRTUAL_IS_OVERRIDDEN(_destroy)){
        _gdvirtual__destroy_call<false>();
    }
}
Dictionary FlatOcTreeOctant::to_dict(){
    Dictionary res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_to_dict)){
        _gdvirtual__to_dict_call<false>(res);
    }else{
        res.get_or_add("center", center);
        res.get_or_add("span", span);
        res.get_or_add("parent", parent);
        res.get_or_add("children", children);
    }
    return res;
}
void FlatOcTreeOctant::copy_from_dict(Dictionary dict){
    //Data less than necessary to reconstruct is permissible
    //as intention may be to store custom data
    if(GDVIRTUAL_IS_OVERRIDDEN(_copy_from_dict)){
        _gdvirtual__copy_from_dict_call<false>(dict);
    }else{
        for(int64_t i = 0; i < dict.keys().size(); i++){
            set(dict.keys()[i], dict[dict.keys()[i]]);
        }
    }
}

