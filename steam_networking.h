#ifndef STEAM_NETWORKING_H
#define STEAM_NETWORKING_H

#include "core/reference.h"
#include "core/io/networked_multiplayer_peer.h"
#include "steam/steam_api.h"

const int CHANNEL = 0;
const int MESSAGE_LIMIT = 250;

class SteamMessagingMultiplayerPeer : public NetworkedMultiplayerPeer {
	GDCLASS(SteamMessagingMultiplayerPeer, NetworkedMultiplayerPeer)

	const int PROTO_SIZE = sizeof(PacketType) + sizeof(uint32_t) + sizeof(int32_t);

	enum PacketType {
		DATA,
		SYS_INIT,
		SYS_SET_ID
	};

	struct Packet {
		PacketType type;
		uint32_t source;
		int32_t destination;
		uint8_t *data;
		uint32_t size;
	};
	
	bool _server;
	int32_t _target_peer;
	uint32_t _peer_id;
	TransferMode _transfer_mode;
	Map<int, SteamNetworkingIdentity> _peer_map;
	List<Packet> _packets;
	bool _refuse_connections;
	ConnectionStatus _connection_status;
	CSteamID* _lobby_id;
	SteamNetworkingMessage_t **_messages;
	uint8_t* make_network_packet(PacketType p_type, uint32_t p_source, int32_t p_destination, const uint8_t *p_buffer, int p_buffer_size);
	Packet make_internal_packet(const uint8_t *p_buffer, int p_buffer_size);

	// Steam callbacks
	void on_lobby_created(LobbyCreated_t *p_callback, bool p_io_failure);
	CCallResult<SteamMessagingMultiplayerPeer, LobbyCreated_t> m_lobby_created_call_result;

	STEAM_CALLBACK(SteamMessagingMultiplayerPeer, on_session_request, SteamNetworkingMessagesSessionRequest_t);
	STEAM_CALLBACK(SteamMessagingMultiplayerPeer, on_lobby_enter, LobbyEnter_t);
	STEAM_CALLBACK(SteamMessagingMultiplayerPeer, on_game_created, LobbyGameCreated_t);
	STEAM_CALLBACK(SteamMessagingMultiplayerPeer, on_lobby_updated, LobbyDataUpdate_t);

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
	void create_lobby(LobbyPrivacy p_lobby_type, int p_max_players);
	void start_server();
	Error join_lobby(uint64_t p_game_id);
	void activate_invite_dialog();

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
	Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	void poll() override;
};

VARIANT_ENUM_CAST(SteamMessagingMultiplayerPeer::LobbyPrivacy);

#endif // STEAM_NETWORKING_H
