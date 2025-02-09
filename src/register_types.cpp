#include "register_types.h"




using namespace godot;

void initialize_static_quadtrees_and_octrees_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(FlatQuadTreeQuadrant);
	GDREGISTER_CLASS(FlatOcTreeOctant);
	GDREGISTER_CLASS(QuadTreeQuadrant);
	GDREGISTER_CLASS(OcTreeOctant);
	GDREGISTER_CLASS(QuadTree);
	GDREGISTER_CLASS(OcTree);
	GDREGISTER_CLASS(ZLocationalCode);
	GDREGISTER_CLASS(ZQuadTree);
	GDREGISTER_CLASS(ZOcTree);
}

void uninitialize_static_quadtrees_and_octrees_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT static_quadtrees_and_octrees_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_static_quadtrees_and_octrees_module);
	init_obj.register_terminator(uninitialize_static_quadtrees_and_octrees_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}