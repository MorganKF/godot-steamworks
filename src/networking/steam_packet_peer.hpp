#ifndef GODOT_STEAMWORKS_STEAM_PACKET_PEER_H
#define GODOT_STEAMWORKS_STEAM_PACKET_PEER_H

#include <godot_cpp/classes/packet_peer_extension.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <queue>
#include <steam/steam_api.h>

using namespace godot;

class SteamPacketPeer : public PacketPeerExtension {
	GDCLASS(SteamPacketPeer, PacketPeerExtension);

private:
	[[maybe_unused]] std::queue<int> _packet_queue;
	SteamNetworkingIdentity _peer {};
	SteamPacketPeer() = default;

protected:
	[[maybe_unused]] static void _bind_methods();

public:
	int64_t _get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) override;
	int64_t _put_packet(const uint8_t *p_buffer, int64_t p_buffer_size) override;
	[[nodiscard]] int64_t _get_available_packet_count() const override;
	[[nodiscard]] int64_t _get_max_packet_size() const override;
    Error send(uint8_t p_channel, const uint8_t *p_buffer, int64_t p_buffer_size, int p_send_flags);

    explicit SteamPacketPeer(SteamNetworkingIdentity p_peer);
    ~SteamPacketPeer() override;
};

#endif // GODOT_STEAMWORKS_STEAM_PACKET_PEER_H
