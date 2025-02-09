#ifndef OCTREE_OCTANT_H
#define OCTREE_OCTANT_H

#include "parted_virtual_macros.h"
#include "flat_octree_octant.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

class FlatOcTreeOctant;
class OcTreeOctant;

class OcTreeOctant : public RefCounted {
    GDCLASS(OcTreeOctant, RefCounted)
    
    friend class OcTree;

private:
    
protected:
    
	static void _bind_methods();

public:
    Vector3 center;
    real_t span;
    OcTreeOctant* parent;
    TypedArray<OcTreeOctant> children;
    //Array children;
    
    OcTreeOctant();
    ~OcTreeOctant();

    //Call this whenever we expect type polymorphed by GDScript/C# implementation
    GDVIRTUAL0R(Ref<OcTreeOctant>, _prototype);
    Ref<OcTreeOctant> prototype();

    GDVIRTUAL0(_destroy);
    void destroy();

    GDVIRTUAL0R_DECLARE(Ref<FlatOcTreeOctant>, _flatten);
    Ref<FlatOcTreeOctant> flatten();

    bool is_root(){ return parent == nullptr; }
    bool is_leaf(){return children.size() == 0;}
    bool contains_point(Vector3 point);
    bool contains_region(PackedVector3Array region);
    
    bool is_descendant_of(Ref<OcTreeOctant> octant);
    bool slab_test(Vector3 origin, Vector3 direction);

    Vector3 get_center(){return center;}
    void set_center(Vector3 _center){center = _center;}
    real_t get_span(){return span;}
    void set_span(real_t _span){span = _span;}
    Ref<OcTreeOctant> get_parent(){return Ref<OcTreeOctant>(parent);}
    void set_parent(Ref<OcTreeOctant> _parent){parent = _parent.ptr();}
    TypedArray<OcTreeOctant> get_children(){return children;}
    void set_children(TypedArray<OcTreeOctant> _children){children = _children;}
    

};

}

#endif