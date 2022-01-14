#include "steam_multiplayer_peer.hpp"

int64_t SteamMultiplayerPeer::_get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) {
	*r_buffer_size = 0;
	return ERR_UNAVAILABLE;
}

int64_t SteamMultiplayerPeer::_put_packet(const uint8_t *p_buffer, int64_t p_buffer_size) {
	return MultiplayerPeerExtension::_put_packet(p_buffer, p_buffer_size);
}

int64_t SteamMultiplayerPeer::_poll() {
	ERR_FAIL_COND_V_MSG(!SteamNetworkingMessages(), ERR_UNAVAILABLE, "Steamworks not initialized.");
	ERR_FAIL_COND_V_MSG(!_is_active(), ERR_UNAVAILABLE, "The multiplayer instance isn't currently active.");

	if (_server)
		_poll_server();
	else
		_poll_client();

	return OK;
}

int64_t SteamMultiplayerPeer::_poll_client() {
	return OK;
}

int64_t SteamMultiplayerPeer::_poll_server() {
	return OK;
}

int64_t SteamMultiplayerPeer::_get_available_packet_count() const {
	return (int64_t)_incoming_packets.size();
}

int64_t SteamMultiplayerPeer::_get_max_packet_size() const {
	return MultiplayerPeerExtension::_get_max_packet_size();
}

void SteamMultiplayerPeer::_set_transfer_channel(int64_t p_channel) {
	_channel = p_channel;
}

int64_t SteamMultiplayerPeer::_get_transfer_channel() const {
	return _channel;
}

void SteamMultiplayerPeer::_set_transfer_mode(int64_t p_mode) {
	_transfer_mode = p_mode;
}

int64_t SteamMultiplayerPeer::_get_transfer_mode() const {
	return _transfer_mode;
}

void SteamMultiplayerPeer::_set_target_peer(int64_t p_peer) {
	_target_peer = p_peer;
}

int64_t SteamMultiplayerPeer::_get_packet_peer() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 1, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(_incoming_packets.empty(), 1);
	return _incoming_packets.front().from;
}

int64_t SteamMultiplayerPeer::_get_unique_id() const {
	return _unique_id;
}

void SteamMultiplayerPeer::_set_refuse_new_connections(bool p_enable) {
	_refuse_new_connections = p_enable;
}

bool SteamMultiplayerPeer::_is_refusing_new_connections() const {
	return _refuse_new_connections;
}

int64_t SteamMultiplayerPeer::_get_connection_status() const {
	return _connection_status;
}

bool SteamMultiplayerPeer::_is_server() const {
	return _server;
}

bool SteamMultiplayerPeer::_is_active() const {
    return _connection_status == CONNECTION_CONNECTED;
}

void SteamMultiplayerPeer::_bind_methods() {
}

SteamMultiplayerPeer::SteamMultiplayerPeer() = default;
SteamMultiplayerPeer::~SteamMultiplayerPeer() = default;
