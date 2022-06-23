#include "register_types.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "networking/steam_multiplayer_peer.hpp"
#include "steamworks.hpp"

using namespace godot;

Ref<Steamworks> steamworks_ref;

void register_godot_steamworks_types(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Steamworks>();
	ClassDB::register_class<SteamPacketPeer>();
	ClassDB::register_class<SteamMultiplayerPeer>();

	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL(engine);

	steamworks_ref.instantiate();
	engine->register_singleton("Steamworks", steamworks_ref.ptr());
}

void unregister_godot_steamworks_types(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	if (steamworks_ref.is_valid()) {
		steamworks_ref.unref();
	}
}

extern "C" GDNativeBool GDN_EXPORT godot_steamworks_init(const GDNativeInterface *p_interface, GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);
	init_obj.register_initializer(register_godot_steamworks_types);
	init_obj.register_terminator(unregister_godot_steamworks_types);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
	return init_obj.init();
}
