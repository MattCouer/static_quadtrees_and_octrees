#ifndef FLAT_QUADTREE_QUADRANT_H
#define FLAT_QUADTREE_QUADRANT_H

#include "quadtree_quadrant.h"
#include "parted_virtual_macros.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

class QuadTreeQuadrant;
class FlatQuadTreeQuadrant;

class FlatQuadTreeQuadrant : public RefCounted {
    GDCLASS(FlatQuadTreeQuadrant, RefCounted)

    friend class QuadTree;

private:


protected:
	static void _bind_methods();

public:

    Vector2 center;
    real_t span;
    int64_t parent;
    PackedInt64Array children;

    FlatQuadTreeQuadrant();
    ~FlatQuadTreeQuadrant();

    GDVIRTUAL0(_destroy);
    void destroy();

    GDVIRTUAL0R(Dictionary, _to_dict);
    GDVIRTUAL1(_copy_from_dict, Dictionary);
    Dictionary to_dict();
    void copy_from_dict(Dictionary dict);


    GDVIRTUAL0R_DECLARE(Ref<QuadTreeQuadrant>, _unflatten);
    Ref<QuadTreeQuadrant> unflatten();

    Vector2 get_center(){return center;}
    void set_center(Vector2 _center){center = _center;}
    real_t get_span(){return span;}
    void set_span(real_t _span){span = _span;}
    
    int64_t get_parent(){ return parent; }
    void set_parent(int64_t _parent) { parent = _parent; }
    PackedInt64Array get_children(){ return children; }
    void set_children(PackedInt64Array _children) { children = _children; }

};

}

#endif