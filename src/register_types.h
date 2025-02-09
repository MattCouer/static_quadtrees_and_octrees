#ifndef STATIC_QUADTREES_AND_OCTREES_REGISTER_TYPES_H
#define STATIC_QUADTREES_AND_OCTREES_REGISTER_TYPES_H

#include "cell_neighbors.h"
#include "quadtree.h"
#include "octree.h"
#include "z_locational_code.h"
#include "z_quadtree.h"
#include "z_octree.h"

#include <godot_cpp/core/class_db.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_static_quadtrees_and_octrees_module(ModuleInitializationLevel p_level);
void uninitialize_static_quadtrees_and_octrees_module(ModuleInitializationLevel p_level);

#endif // STATIC_QUADTREES_AND_OCTREES_REGISTER_TYPES_H