#include <cstdlib>
#include <string>

#include "steamworks.h"


Steamworks* Steamworks::_singleton;

Steamworks::Steamworks() {
	_singleton = this;
}

Steamworks::~Steamworks() {
	SteamAPI_Shutdown();
}

Steamworks* Steamworks::get_singleton() {
	return _singleton;
}

void Steamworks::init(const int p_app_id) const {
	#ifdef TOOLS_ENABLED
		const std::string app_id = std::to_string(p_app_id);
		_putenv_s("SteamAppId", app_id.c_str());
		_putenv_s("SteamGameId", app_id.c_str());
	#endif
	SteamAPI_Init();
}

void Steamworks::run_callbacks() const {
	SteamAPI_RunCallbacks();
}

uint64_t Steamworks::get_steam_id() const {
	if (SteamUser() == nullptr) {
		return 0;
	}
	return SteamUser()->GetSteamID().ConvertToUint64();
}

void Steamworks::on_join_request(GameLobbyJoinRequested_t *p_callback) {
	emit_signal("join_request", p_callback->m_steamIDLobby.ConvertToUint64());
}

void Steamworks::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "app_id"), &Steamworks::init);
	ClassDB::bind_method(D_METHOD("get_steam_id"), &Steamworks::get_steam_id);
	ClassDB::bind_method(D_METHOD("run_callbacks"), &Steamworks::run_callbacks);

	ADD_SIGNAL(MethodInfo("join_request", PropertyInfo(Variant::INT, "game_id")));
}
