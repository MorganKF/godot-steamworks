#include "register_types.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "steamworks.hpp"

using namespace godot;

Ref<Steamworks> steamworks_ref;

void register_godot_steamworks_types() {
	ClassDB::register_class<Steamworks>();
	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL(engine);

	steamworks_ref.instantiate();
	engine->register_singleton("Steamworks", steamworks_ref.ptr());
}

void unregister_godot_steamworks_types() {
	if (steamworks_ref.is_valid()) {
		steamworks_ref.unref();
	}
}

extern "C" {
GDNativeBool GDN_EXPORT godot_steamworks_init(const GDNativeInterface *p_interface, const GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

	init_obj.register_scene_initializer(register_godot_steamworks_types);
	init_obj.register_scene_terminator(unregister_godot_steamworks_types);

	return init_obj.init();
}
}
