#ifndef QUADTREE_QUADRANT_H
#define QUADTREE_QUADRANT_H

#include "parted_virtual_macros.h"
#include "flat_quadtree_quadrant.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

class FlatQuadTreeQuadrant;
class QuadTreeQuadrant;

class QuadTreeQuadrant : public RefCounted {
    GDCLASS(QuadTreeQuadrant, RefCounted)

    friend class QuadTree;

private:


protected:
	static void _bind_methods();

public:
    Vector2 center;
    real_t span;
    QuadTreeQuadrant* parent;
    TypedArray<QuadTreeQuadrant> children;
    //Array children;

    QuadTreeQuadrant();
    ~QuadTreeQuadrant();
    //Call this whenever we expect type polymorphed by GDScript/C# implementation
    GDVIRTUAL0R(Ref<QuadTreeQuadrant>, _prototype);
    Ref<QuadTreeQuadrant> prototype();

    GDVIRTUAL0R_DECLARE(Ref<FlatQuadTreeQuadrant>, _flatten);
    Ref<FlatQuadTreeQuadrant> flatten();

    GDVIRTUAL0(_destroy);
    void destroy();

    bool is_root() { return parent == nullptr; }
    bool is_leaf() { return children.size() == 0; }
    bool contains_point(Vector2 point);
    bool contains_region(PackedVector2Array region);

    bool is_descendant_of(Ref<QuadTreeQuadrant> quadrant);
    bool is_intersected_by_ray(Vector2 ray_origin, Vector2 ray_direction);
    bool slab_test(Vector2 origin, Vector2 direction);

    Vector2 get_center(){return center;}
    void set_center(Vector2 _center){center = _center;}
    real_t get_span(){return span;}
    void set_span(real_t _span){span = _span;}
    Ref<QuadTreeQuadrant> get_parent(){return Ref<QuadTreeQuadrant>(parent);}
    void set_parent(Ref<QuadTreeQuadrant> _parent){parent = _parent.ptr();}
    TypedArray<QuadTreeQuadrant> get_children(){return children;}
    void set_children(TypedArray<QuadTreeQuadrant> _children){children = _children;}

};

}

#endif