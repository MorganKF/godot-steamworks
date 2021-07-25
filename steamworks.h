#ifndef GODOT_STEAMWORKS_H
#define GODOT_STEAMWORKS_H

#include "core/reference.h"
#include "steam/steam_api.h"

class Steamworks : public Reference {
	GDCLASS(Steamworks, Reference);
	STEAM_CALLBACK(Steamworks, on_join_request, GameLobbyJoinRequested_t);

protected:
	static void _bind_methods();
	static Steamworks* _singleton;

public:
	Steamworks();
	~Steamworks() override;
	static Steamworks* get_singleton();
	void init(int p_app_id) const;
	void run_callbacks() const;
	uint64_t get_steam_id() const;
};
#endif
