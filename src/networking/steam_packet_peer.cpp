#include "steam_packet_peer.hpp"

[[maybe_unused]] void SteamPacketPeer::_bind_methods() {
}

// Is this used by Godot?
int64_t SteamPacketPeer::_get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) {
	WARN_PRINT_ONCE("Get packet not exposed for steam networking messages peers.");
	return PacketPeerExtension::_get_packet(r_buffer, r_buffer_size);
}

int64_t SteamPacketPeer::_put_packet(const uint8_t *p_buffer, int64_t p_buffer_size) {
	return send(0, p_buffer, p_buffer_size, k_nSteamNetworkingSend_Reliable);
}

Error SteamPacketPeer::send(uint8_t p_channel, const uint8_t *p_buffer, int64_t p_buffer_size, int p_send_flags) {
    SteamNetworkingMessages()->SendMessageToUser(_peer, p_buffer, p_buffer_size, p_send_flags, 0);
	return OK;
}

int64_t SteamPacketPeer::_get_available_packet_count() const {
	return PacketPeerExtension::_get_available_packet_count();
}

int64_t SteamPacketPeer::_get_max_packet_size() const {
	return 1 << 24;
}

SteamPacketPeer::SteamPacketPeer(SteamNetworkingIdentity p_peer) {
    _peer = p_peer;
}

SteamPacketPeer::~SteamPacketPeer() = default;
