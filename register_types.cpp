#include "register_types.h"

#include "core/engine.h"
#include "core/class_db.h"
#include "godot_steamworks.h"
#include "steam_networking.h"

static GodotSteamworks* godot_steamworks_ptr = nullptr;

void register_godot_steamworks_types() {
	ClassDB::register_class<GodotSteamworks>();
	godot_steamworks_ptr = memnew(GodotSteamworks);
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotSteamworks", GodotSteamworks::get_singleton()));

	ClassDB::register_class<SteamMessagingMultiplayerPeer>();
}

void unregister_godot_steamworks_types() {
	memdelete(godot_steamworks_ptr);
}

