#ifndef GODOT_STEAMWORKS_H
#define GODOT_STEAMWORKS_H

#include "core/reference.h"
#include "steam/steam_api.h"

class GodotSteamworks : public Reference {
GDCLASS(GodotSteamworks, Reference)

	STEAM_CALLBACK(GodotSteamworks, on_join_request, GameLobbyJoinRequested_t);

protected:
	static void _bind_methods();
	static GodotSteamworks* singleton_;

public:
	GodotSteamworks();
	~GodotSteamworks() override;
	static GodotSteamworks* get_singleton();
	void init(int p_app_id) const;
	void run_callbacks() const;
	uint64_t get_steam_id() const;
};
#endif
