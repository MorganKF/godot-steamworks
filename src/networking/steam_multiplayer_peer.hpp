#ifndef GODOT_STEAMWORKS_STEAM_MULTIPLAYER_PEER_HPP
#define GODOT_STEAMWORKS_STEAM_MULTIPLAYER_PEER_HPP

#include "steam_packet_peer.hpp"

#include <godot_cpp/classes/multiplayer_peer_extension.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <queue>
#include <steam/steam_api.h>

using namespace godot;

class SteamMultiplayerPeer : public MultiplayerPeerExtension {
	GDCLASS(SteamMultiplayerPeer, MultiplayerPeerExtension);
	const int MESSAGE_LIMIT = 60;
	const uint32_t HEADER_SIZE = sizeof(Type) + sizeof(int64_t) * 2;

private:
	enum Type {
		DATA,
	};

	struct Packet {
		SteamNetworkingMessage_t *message = nullptr;
		int64_t from = 0;
		int64_t to = 0;
	};

	bool _server = false;
	bool _server_relay = true;
	bool _lobby = false;
	bool _refuse_new_connections = false;
	uint32_t _unique_id = 0;
	int64_t _target_peer = 0;
	int64_t _channel = 0;
	int64_t _transfer_mode = 2;
	ConnectionStatus _connection_status = CONNECTION_DISCONNECTED;
	std::queue<Packet> _incoming_packets;
	Packet _current_packet;
	std::unordered_map<int, Ref<SteamPacketPeer>> _peers;
	SteamNetworkingMessage_t **_messages;

	int64_t _poll_client();
	int64_t _poll_server();

protected:
	static void _bind_methods();

public:
	[[nodiscard]] bool _is_server() const override;
	int64_t _get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) override;
	int64_t _put_packet(const uint8_t *p_buffer, int64_t p_buffer_size) override;
	[[nodiscard]] int64_t _get_available_packet_count() const override;
	[[nodiscard]] int64_t _get_max_packet_size() const override;
	void _set_transfer_channel(int64_t p_channel) override;
	[[nodiscard]] int64_t _get_transfer_channel() const override;
	void _set_transfer_mode(int64_t p_mode) override;
	[[nodiscard]] int64_t _get_transfer_mode() const override;
	void _set_target_peer(int64_t p_peer) override;
	[[nodiscard]] int64_t _get_packet_peer() const override;
	int64_t _poll() override;
	[[nodiscard]] int64_t _get_unique_id() const override;
	void _set_refuse_new_connections(bool p_enable) override;
	bool _is_refusing_new_connections() const override;
	[[nodiscard]] int64_t _get_connection_status() const override;
    [[nodiscard]] bool _is_active() const;
	SteamMultiplayerPeer();
	~SteamMultiplayerPeer() override;
};

#endif // GODOT_STEAMWORKS_STEAM_MULTIPLAYER_PEER_HPP
