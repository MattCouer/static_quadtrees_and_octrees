#include "quadtree_quadrant.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void QuadTreeQuadrant::_bind_methods(){
    ClassDB::bind_method(D_METHOD("is_root"), &QuadTreeQuadrant::is_root);
	ClassDB::bind_method(D_METHOD("is_leaf"), &QuadTreeQuadrant::is_leaf);
    ClassDB::bind_method(D_METHOD("contains_point", "point"), &QuadTreeQuadrant::contains_point);
	ClassDB::bind_method(D_METHOD("contains_region", "region"), &QuadTreeQuadrant::contains_region);
    ClassDB::bind_method(D_METHOD("get_center"), &QuadTreeQuadrant::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &QuadTreeQuadrant::set_center);
    ClassDB::bind_method(D_METHOD("get_span"), &QuadTreeQuadrant::get_span);
	ClassDB::bind_method(D_METHOD("set_span", "span"), &QuadTreeQuadrant::set_span);
    ClassDB::bind_method(D_METHOD("get_parent"), &QuadTreeQuadrant::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "parent"), &QuadTreeQuadrant::set_parent);
    ClassDB::bind_method(D_METHOD("get_children"), &QuadTreeQuadrant::get_children);
	ClassDB::bind_method(D_METHOD("set_children", "children"), &QuadTreeQuadrant::set_children);
    
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "center"), "set_center", "get_center");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "span"), "set_span", "get_span");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children"), "set_children", "get_children");

    //VIRTUAL METHODS
    GDVIRTUAL_BIND(_flatten);
    GDVIRTUAL_BIND(_prototype);
    GDVIRTUAL_BIND(_destroy);

}

//VIRTUAL DEFINITIONS
GDVIRTUAL0R_DEFINE(FlatQuadTreeQuadrant, Ref<QuadTreeQuadrant>, _unflatten);
//To be only called by QuadTree::flat_to_linear_recursive()
Ref<QuadTreeQuadrant> FlatQuadTreeQuadrant::unflatten(){
    Ref<QuadTreeQuadrant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_unflatten)){
        _gdvirtual__unflatten_call<false>(res);
        
    }
    if(res.is_null())
        res.instantiate();
    res->center = center;
    res->span = span;
    return res;
}


QuadTreeQuadrant::QuadTreeQuadrant(){
    parent = nullptr;
}
QuadTreeQuadrant::~QuadTreeQuadrant(){
    destroy();
}


Ref<QuadTreeQuadrant> QuadTreeQuadrant::prototype(){
    Ref<QuadTreeQuadrant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_prototype)){
        _gdvirtual__prototype_call<false>(res);
    }
    if(res.is_null())
        res.instantiate();
    return res;
}

void QuadTreeQuadrant::destroy(){
    if(GDVIRTUAL_IS_OVERRIDDEN(_destroy)){
        _gdvirtual__destroy_call<false>();
    }
}

bool QuadTreeQuadrant::contains_point(Vector2 point){
    return (point.x >= center.x-span && point.x <= center.x + span
        && point.y >= center.y - span && point.y <= center.y + span);
}
bool QuadTreeQuadrant::contains_region(PackedVector2Array region){
    for(Vector2 v : region)
        if(!contains_point(v))
            return false;
    return true;
}


bool QuadTreeQuadrant::is_descendant_of(Ref<QuadTreeQuadrant> quadrant){
    if(is_root())
        return false;

    Ref<QuadTreeQuadrant> position = parent;
    while(!(position->is_root())){
        if(position == quadrant)
            return true;
        position = Ref<QuadTreeQuadrant>(position->parent);
    }

    return false;
}

bool QuadTreeQuadrant::is_intersected_by_ray(Vector2 ray_origin, Vector2 ray_direction){
    return true;
}


bool QuadTreeQuadrant::slab_test(Vector2 origin, Vector2 direction){
    real_t tmin = std::numeric_limits<real_t>::min();
    real_t tmax = std::numeric_limits<real_t>::max();

    Vector2 aa_min = center - Vector2(span, span);
    Vector2 aa_max = center + Vector2(span, span);

    for(int64_t i = 0; i < 2; i++){
        real_t dir = direction[i];
        real_t ori = origin[i];

        real_t aa_min_c = aa_min[i];
        real_t aa_max_c = aa_max[i];

        if(fabs(dir) < 0.000001){
            if(ori < aa_min_c || ori > aa_max_c){
                return false;
            }
        }else{
            real_t t1 = (aa_min_c - ori)/dir;
            real_t t2 = (aa_max_c - ori)/dir;
            tmin = std::max(tmin, std::min(t1, t2));
            tmax = std::min(tmax, std::max(t1, t2));
        }

        if(tmin > tmax)
            return false;
    }
    
    if(tmax < 0)
        return false;

    return true;

}




