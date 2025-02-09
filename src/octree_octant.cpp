#include "octree_octant.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void OcTreeOctant::_bind_methods(){
    ClassDB::bind_method(D_METHOD("is_root"), &OcTreeOctant::is_root);
	ClassDB::bind_method(D_METHOD("is_leaf"), &OcTreeOctant::is_leaf);
    ClassDB::bind_method(D_METHOD("contains_point"), &OcTreeOctant::contains_point);
	ClassDB::bind_method(D_METHOD("contains_region"), &OcTreeOctant::contains_region);
    ClassDB::bind_method(D_METHOD("get_center"), &OcTreeOctant::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &OcTreeOctant::set_center);
    ClassDB::bind_method(D_METHOD("get_span"), &OcTreeOctant::get_span);
	ClassDB::bind_method(D_METHOD("set_span", "span"), &OcTreeOctant::set_span);
    ClassDB::bind_method(D_METHOD("get_parent"), &OcTreeOctant::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "parent"), &OcTreeOctant::set_parent);
    ClassDB::bind_method(D_METHOD("get_children"), &OcTreeOctant::get_children);
	ClassDB::bind_method(D_METHOD("set_children", "children"), &OcTreeOctant::set_children);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "span"), "set_span", "get_span");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children"), "set_children", "get_children");

    //VIRTUAL METHODS
    GDVIRTUAL_BIND(_flatten);
    GDVIRTUAL_BIND(_prototype);
    GDVIRTUAL_BIND(_destroy);

}

//VIRTUAL DEFINITIONS
GDVIRTUAL0R_DEFINE(FlatOcTreeOctant, Ref<OcTreeOctant>, _unflatten);
//To be only called by OcTree::flat_to_linear_recursive()
Ref<OcTreeOctant> FlatOcTreeOctant::unflatten(){
    Ref<OcTreeOctant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_unflatten)){
        _gdvirtual__unflatten_call<false>(res);
    }
    if(res.is_null())
        res.instantiate();
    res->center = center;
    res->span = span;
    return res;
}

OcTreeOctant::OcTreeOctant(){
    parent = nullptr;
}
OcTreeOctant::~OcTreeOctant(){
    destroy();
}

Ref<OcTreeOctant> OcTreeOctant::prototype(){
    Ref<OcTreeOctant> res;
    if(GDVIRTUAL_IS_OVERRIDDEN(_prototype)){
        _gdvirtual__prototype_call<false>(res);
    }
    if(res.is_null())
        res.instantiate();
    return res;
}

void OcTreeOctant::destroy(){
    if(GDVIRTUAL_IS_OVERRIDDEN(_destroy)){
        _gdvirtual__destroy_call<false>();
    }
}

bool OcTreeOctant::contains_point(Vector3 point){
    return (point.x >= center.x-span && point.x <= center.x + span
        && point.y >= center.y - span && point.y <= center.y + span
        && point.z >= center.z - span && point.z <= center.z + span);
}
bool OcTreeOctant::contains_region(PackedVector3Array region){
    for(Vector3 v : region)
        if(!contains_point(v))
            return false;
    return true;
}

bool OcTreeOctant::is_descendant_of(Ref<OcTreeOctant> octant){
    if(is_root())
        return false;

    Ref<OcTreeOctant> position = parent;
    while(!(position->is_root())){
        if(position == octant)
            return true;
        position = Ref<OcTreeOctant>(position->parent);
    }
    return false;
}


bool OcTreeOctant::slab_test(Vector3 origin, Vector3 direction){
    real_t tmin = std::numeric_limits<real_t>::min();
    real_t tmax = std::numeric_limits<real_t>::max();

    Vector3 aa_min = center - Vector3(span, span, span);
    Vector3 aa_max = center + Vector3(span, span, span);

    for(int64_t i = 0; i < 3; i++){
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
    return (tmax >= tmin) && (tmax >= 0);
}