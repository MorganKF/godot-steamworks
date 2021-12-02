#include "register_types.h"

#include "src/steamworks.h"
#include "src/steam_networking.h"
#include "core/config/engine.h"

static Steamworks *godot_steamworks_ptr = nullptr;

void register_steamworks_types() {
	GDREGISTER_CLASS(Steamworks)
	godot_steamworks_ptr = memnew(Steamworks);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Steamworks", Steamworks::get_singleton()));

	// Todo: Update to new networking stack
	//GDREGISTER_VIRTUAL_CLASS(SteamMessagingMultiplayerPeer)
}

void unregister_steamworks_types() {
	memdelete(godot_steamworks_ptr);
}
