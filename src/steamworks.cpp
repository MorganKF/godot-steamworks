#include "steamworks.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <steam/steam_api.h>

void Steamworks::init() const {
	SteamAPI_Init();
}

void Steamworks::run_callbacks() const {
	SteamAPI_RunCallbacks();
}

void Steamworks::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init"), &Steamworks::init);
    ClassDB::bind_method(D_METHOD("run_callbacks"), &Steamworks::run_callbacks);
}

Steamworks::Steamworks() = default;

Steamworks::~Steamworks() {
    SteamAPI_Shutdown();
}
