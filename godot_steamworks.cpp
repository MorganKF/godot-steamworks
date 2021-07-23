#include <cstdlib>
#include <string>

#include "godot_steamworks.h"


GodotSteamworks *GodotSteamworks::singleton_;

GodotSteamworks::GodotSteamworks() {
	singleton_ = this;
}

GodotSteamworks::~GodotSteamworks() {
	SteamAPI_Shutdown();
}

GodotSteamworks *GodotSteamworks::get_singleton() {
	return singleton_;
}

void GodotSteamworks::init(const int p_app_id) const {
	#ifdef TOOLS_ENABLED
		const std::string app_id = std::to_string(p_app_id);
		_putenv_s("SteamAppId", app_id.c_str());
		_putenv_s("SteamGameId", app_id.c_str());
	#endif
	SteamAPI_Init();
}

void GodotSteamworks::run_callbacks() const {
	SteamAPI_RunCallbacks();
}

uint64_t GodotSteamworks::get_steam_id() const {
	return SteamUser()->GetSteamID().ConvertToUint64();
}

void GodotSteamworks::on_join_request(GameLobbyJoinRequested_t *p_callback) {
	emit_signal("join_request", p_callback->m_steamIDLobby.ConvertToUint64());
}

void GodotSteamworks::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "app_id"), &GodotSteamworks::init);
	ClassDB::bind_method(D_METHOD("get_steam_id"), &GodotSteamworks::get_steam_id);
	ClassDB::bind_method(D_METHOD("run_callbacks"), &GodotSteamworks::run_callbacks);

	ADD_SIGNAL(MethodInfo("join_request", PropertyInfo(Variant::INT, "game_id")));
}
