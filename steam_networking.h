#ifndef STEAM_NETWORKING_H
#define STEAM_NETWORKING_H

#include "core/reference.h"
#include "core/io/networked_multiplayer_peer.h"
#include "steam/steam_api.h"

const int CHANNEL = 0;
const int PROTO_SIZE = 9;

class SteamMessagingMultiplayerPeer : public NetworkedMultiplayerPeer {
	GDCLASS(SteamMessagingMultiplayerPeer, NetworkedMultiplayerPeer)

	enum PacketType {
		DATA,
		HANDSHAKE,
	};

	struct Packet {
		PacketType type;
		int source;
		int destination;
		uint8_t *data;
		uint32_t size;
	};
	
	bool _server;
	int _target_peer;
	int _peer_id;
	TransferMode _transfer_mode;
	Map<int, SteamNetworkingIdentity> _peer_map;
	List<Packet> _packets;
	bool _refuse_connections;
	ConnectionStatus _connection_status;
	CSteamID* _lobby_id;
	PoolVector<uint8_t> make_packet(PacketType p_type, uint32_t p_source, uint32_t p_destination, const uint8_t *p_buffer, int p_buffer_size);
	Packet break_packet(const uint8_t *p_buffer, int p_buffer_size);

	// Steam callbacks
	void on_lobby_created(LobbyCreated_t *p_callback, bool p_io_failure);
	CCallResult<SteamMessagingMultiplayerPeer, LobbyCreated_t> m_lobby_created_call_result;

	STEAM_CALLBACK(SteamMessagingMultiplayerPeer, on_lobby_enter, LobbyCreated_t);

protected:
	static void _bind_methods();

public:
	enum LobbyPrivacy {
		OPEN,
		FRIENDS,
		CLOSED
	};

	SteamMessagingMultiplayerPeer();
	~SteamMessagingMultiplayerPeer() override;

	void create_server(LobbyPrivacy p_lobby_type, int p_max_players);
	// Error connect(CSteamID p_game_id);

	/* NetworkedMultiplayerPeer */
	void set_transfer_mode(TransferMode p_mode) override;
	TransferMode get_transfer_mode() const override;
	void set_target_peer(int p_peer_id) override;
	int get_packet_peer() const override;
	int get_unique_id() const override;
	bool is_server() const override { return _server; }
	void set_refuse_new_connections(bool p_enable) override;
	bool is_refusing_new_connections() const override;
	ConnectionStatus get_connection_status() const override;

	/* PacketPeer */
	int get_available_packet_count() const override;
	int get_max_packet_size() const override;
	void activate_invite_dialog();
	Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	void poll() override;
};

VARIANT_ENUM_CAST(SteamMessagingMultiplayerPeer::LobbyPrivacy);

#endif // STEAM_NETWORKING_H
