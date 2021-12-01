#include "register_types.h"

#include "core/engine.h"
#include "core/class_db.h"
#include "src/steamworks.h"
#include "src/steam_networking.h"

static Steamworks* godot_steamworks_ptr = nullptr;

void register_godot_steamworks_types() {
	ClassDB::register_class<Steamworks>();
	godot_steamworks_ptr = memnew(Steamworks);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Steamworks", Steamworks::get_singleton()));

	ClassDB::register_class<SteamMessagingMultiplayerPeer>();
}

void unregister_godot_steamworks_types() {
	memdelete(godot_steamworks_ptr);
}

