#include "flat_quadtree_quadrant.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FlatQuadTreeQuadrant::_bind_methods(){
    ClassDB::bind_method(D_METHOD("get_center"), &FlatQuadTreeQuadrant::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &FlatQuadTreeQuadrant::set_center);
    ClassDB::bind_method(D_METHOD("get_span"), &FlatQuadTreeQuadrant::get_span);
	ClassDB::bind_method(D_METHOD("set_span", "span"), &FlatQuadTreeQuadrant::set_span);
	ClassDB::bind_method(D_METHOD("get_parent"), &FlatQuadTreeQuadrant::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "parent"), &FlatQuadTreeQuadrant::set_parent);
	ClassDB::bind_method(D_METHOD("get_children"), &FlatQuadTreeQuadrant::get_children);
	ClassDB::bind_method(D_METHOD("set_children", "children"), &FlatQuadTreeQuadrant::set_children);
    
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "center"), "set_center", "get_center");
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
GDVIRTUAL0R_DEFINE(QuadTreeQuadrant, Ref<FlatQuadTreeQuadrant>, _flatten);
//To be only called by QuadTree::flatten()
Ref<FlatQuadTreeQuadrant> QuadTreeQuadrant::flatten(){
    Ref<FlatQuadTreeQuadrant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_flatten)){
        _gdvirtual__flatten_call<false>(res);
    }
    if(res.is_null())
        res.instantiate();
    res->center = center;
    res->span = span;
    res->children.resize(4);
    if(is_root())
        res->parent = -1;
    if(is_leaf())
        res->children.fill(-1);
    return res;
}

::godot::FlatQuadTreeQuadrant::FlatQuadTreeQuadrant(){
}
FlatQuadTreeQuadrant::~FlatQuadTreeQuadrant(){
    destroy();
}
void FlatQuadTreeQuadrant::destroy(){
    if(GDVIRTUAL_IS_OVERRIDDEN(_destroy)){
        _gdvirtual__destroy_call<false>();
    }
}
Dictionary FlatQuadTreeQuadrant::to_dict(){
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
void FlatQuadTreeQuadrant::copy_from_dict(Dictionary dict){
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