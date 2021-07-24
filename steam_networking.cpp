#include "steam_networking.h"
#include "core/io/marshalls.h"

SteamMessagingMultiplayerPeer::SteamMessagingMultiplayerPeer() :
		_server(false),
		_target_peer(TARGET_PEER_BROADCAST),
		_transfer_mode(TRANSFER_MODE_RELIABLE),
		_refuse_connections(true),
		_connection_status(CONNECTION_DISCONNECTED),
		_lobby_id(nullptr) {
}

SteamMessagingMultiplayerPeer::~SteamMessagingMultiplayerPeer() {
	memfree(_lobby_id);
}

void SteamMessagingMultiplayerPeer::set_transfer_mode(TransferMode p_mode) {
	_transfer_mode = p_mode;
}

NetworkedMultiplayerPeer::TransferMode SteamMessagingMultiplayerPeer::get_transfer_mode() const {
	return _transfer_mode;
}

void SteamMessagingMultiplayerPeer::set_target_peer(int p_peer_id) {
	_target_peer = p_peer_id;
}

int SteamMessagingMultiplayerPeer::get_packet_peer() const {
	ERR_FAIL_COND_V(_packets.empty(), 1)
	return _packets.front()->get().source;
}

int SteamMessagingMultiplayerPeer::get_unique_id() const {
	return _peer_id;
}

void SteamMessagingMultiplayerPeer::set_refuse_new_connections(bool p_enable) {
	_refuse_connections = p_enable;
}

bool SteamMessagingMultiplayerPeer::is_refusing_new_connections() const {
	return _refuse_connections;
}

NetworkedMultiplayerPeer::ConnectionStatus SteamMessagingMultiplayerPeer::get_connection_status() const {
	return _connection_status;
}

int SteamMessagingMultiplayerPeer::get_available_packet_count() const {
	return _packets.size();
}

int SteamMessagingMultiplayerPeer::get_max_packet_size() const {
	return std::numeric_limits<int>::max();
}

void SteamMessagingMultiplayerPeer::activate_invite_dialog() {
	if (_lobby_id == nullptr) {
		WARN_PRINT("Attempting to open invite overlay without initializing lobby!");
		return;
	}

	SteamFriends()->ActivateGameOverlayInviteDialog(*_lobby_id);
};

PoolVector<uint8_t> SteamMessagingMultiplayerPeer::make_network_packet(PacketType p_type, uint32_t p_source, uint32_t p_destination, const uint8_t *p_buffer, int p_buffer_size) {
	PoolVector<uint8_t> packet;
	packet.resize(p_buffer_size + PROTO_SIZE);
	const auto writer = packet.write();
	memcpy(&writer[0], &p_type, 1);
	memcpy(&writer[1], &p_source, 4);
	memcpy(&writer[1], &p_destination, 4);
	memcpy(&writer[PROTO_SIZE], p_buffer, p_buffer_size);
	return packet;
}

SteamMessagingMultiplayerPeer::Packet SteamMessagingMultiplayerPeer::make_internal_packet(const uint8_t *p_buffer, int p_buffer_size) {
	Packet packet{};
	packet.size = p_buffer_size;
	packet.data = (uint8_t *)(memalloc(packet.size));
	memcpy(&packet.type, p_buffer, 1);
	memcpy(&packet.source, &p_buffer[1], 4);
	memcpy(&packet.destination, &p_buffer[4], 4);
	memcpy(&packet.data, &p_buffer[PROTO_SIZE], p_buffer_size);
	return packet;
}

// Creates steam lobby
void SteamMessagingMultiplayerPeer::create_lobby(LobbyPrivacy p_lobby_type, int p_max_players) {
	if (SteamMatchmaking() == nullptr) {
		ERR_PRINT("SteamAPI has not been initialized!");
		return;
	}

	_connection_status = CONNECTION_CONNECTING;
	const SteamAPICall_t api_call = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, p_max_players);
	m_lobby_created_call_result.Set(api_call, this, &SteamMessagingMultiplayerPeer::on_lobby_created);
}

// Moves lobby to server
void SteamMessagingMultiplayerPeer::start_server() {
	_peer_id = 1;
	_refuse_connections = false;
	_connection_status = CONNECTION_CONNECTED;
	SteamMatchmaking()->SetLobbyGameServer(*_lobby_id, 0, 0, SteamUser()->GetSteamID());
}

// Connectes client to server
Error SteamMessagingMultiplayerPeer::join_lobby(uint64_t p_game_id) {
	if (SteamMatchmaking() == nullptr) {
		ERR_PRINT("SteamAPI has not been initialized!");
		return ERR_CANT_ACQUIRE_RESOURCE;
	}
	_connection_status = CONNECTION_CONNECTED;
	SteamMatchmaking()->JoinLobby(CSteamID(p_game_id));
	return OK;
}

void SteamMessagingMultiplayerPeer::on_lobby_created(LobbyCreated_t *p_callback, bool p_io_failure) {
	if (p_callback->m_eResult == k_EResultOK) {
		print_line("Lobby created!");
		print_line(itos(p_callback->m_ulSteamIDLobby));
		_lobby_id = (CSteamID*)memalloc(sizeof(CSteamID));
		*_lobby_id = CSteamID(p_callback->m_ulSteamIDLobby);
		_server = true;
	} else {
		// Todo
	}
}

Error SteamMessagingMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	Packet packet = _packets.front()->get();
	_packets.pop_front();

	*r_buffer = packet.data;
	r_buffer_size = packet.size;

	return OK;
}

Error SteamMessagingMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	if (SteamNetworking() == nullptr) {
		return ERR_UNAVAILABLE;
	}

	PoolVector<uint8_t> packet = make_network_packet(DATA, get_unique_id(), _target_peer, p_buffer, p_buffer_size);

	int flags = 0;
	switch (_transfer_mode) {
		case TRANSFER_MODE_UNRELIABLE: {
			flags = k_nSteamNetworkingSend_UnreliableNoDelay;
		} break;
		case TRANSFER_MODE_RELIABLE: {
			flags = k_nSteamNetworkingSend_Reliable;
		} break;
		case TRANSFER_MODE_UNRELIABLE_ORDERED: {
			flags = k_nSteamNetworkingSend_Unreliable;
		}
	}

	if (is_server()) {
		if (_target_peer == 1) {
			return OK; // Don't sent to self
		} else if (_target_peer == 0) {
			// Send to everyone
		} else if (_target_peer < 0) {
			// Send to all excluding one
		} else {
			// Send to target
			const auto result = SteamNetworkingMessages()->SendMessageToUser(_peer_map[_target_peer], &packet, p_buffer_size, flags, CHANNEL);

			if (result == k_EResultNoConnection) {
				return ERR_CANT_CONNECT;
			}
		}
	} else {
		// We are a client so messages can only go to the server
		const auto result = SteamNetworkingMessages()->SendMessageToUser(_peer_map[1], &packet, p_buffer_size, flags, CHANNEL);

		if (result == k_EResultNoConnection) {
			return ERR_CANT_CONNECT;
		}
	}

	return OK;
}

void SteamMessagingMultiplayerPeer::poll() {
	// Get lastest network messages
	SteamNetworkingMessage_t **messages = (SteamNetworkingMessage_t**)memalloc(sizeof(SteamNetworkingMessage_t) * 250);
	int num_messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(CHANNEL, messages, 250);

	for (auto i = 0; i < num_messages; i++) {
		// Unpack message
		const uint8_t * data = (uint8_t*)messages[i]->m_pData;
		int size = messages[i]->m_cbSize;
		auto packet = make_internal_packet(data, size);

		switch (packet.type) {
			case PacketType::DATA: {
				_packets.push_back(packet); // Normal Godot packets
			} break;

			// Used to set client unique ids
			case PacketType::SYS_SET_ID: {
				if (packet.source == 1) {
					print_line("Got ID from server!");
					emit_signal("connection_succeeded");
					emit_signal("peer_connected", packet.destination);
					_peer_id = packet.destination;
					_connection_status = CONNECTION_CONNECTED;
				}
			} break;

			// Initializes internal data for server peers
			case PacketType::SYS_INIT: {
				if (_server) {
					print_line("Player connected!");
					// Generate ID
					int id;
					if (_peer_map.size() == 0) {
						id = 2;
					} else {
						id = _peer_map.back()->key() + 1;
					}
					_peer_map[id] = messages[i]->m_identityPeer;

					// Emit Godot signals
					emit_signal("peer_connected", id);
					emit_signal("connected_to_server");

					// Send ID to new peer
					auto packet = make_network_packet(SYS_SET_ID, _peer_id, id, nullptr, 0); // Will this work?
					SteamNetworkingMessages()->SendMessageToUser(messages[i]->m_identityPeer, &packet, 0, k_nSteamNetworkingSend_Reliable, CHANNEL);
				} else {
					WARN_PRINT("GOT SIS_INIT FROM PLAYER?");
				}
			} break;
		}

		// Free steam data
		messages[i]->Release();
	}
}

void SteamMessagingMultiplayerPeer::on_lobby_update(LobbyDataUpdate_t *p_callback) {
	CSteamID id = CSteamID(p_callback->m_ulSteamIDMember);
	
	if (_server && id != *_lobby_id) {
		print_line("Player joined lobby!");
		print_line(itos(id.ConvertToUint64()));
	}

	if (!_server && id.IsLobby() && _lobby_id == nullptr) {
		_lobby_id = (CSteamID *)memalloc(sizeof(CSteamID));
		*_lobby_id = id;
		print_line("Joined a lobby!");
	}
}

void SteamMessagingMultiplayerPeer::on_session_request(SteamNetworkingMessagesSessionRequest_t *p_callback) {
	print_line(vformat("Session request from: %i", p_callback->m_identityRemote.GetSteamID64()));
	if (!_refuse_connections) {
		SteamNetworkingMessages()->AcceptSessionWithUser(p_callback->m_identityRemote);
	}
}

void SteamMessagingMultiplayerPeer::on_game_created(LobbyGameCreated_t* p_callback) {
	// Todo: check if steam id is actually set
	if (!_server) {
		auto packet = make_network_packet(SYS_INIT, 0, 1, nullptr, 0); // Will this work?
		auto id = SteamNetworkingIdentity();
		id.SetSteamID64(p_callback->m_ulSteamIDGameServer);
		SteamNetworkingMessages()->SendMessageToUser(id, &packet, 0, k_nSteamNetworkingSend_Reliable, CHANNEL);
	}
}

void SteamMessagingMultiplayerPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_lobby", "lobby_type", "max_players"), &SteamMessagingMultiplayerPeer::create_lobby);
	ClassDB::bind_method(D_METHOD("join_lobby"), &SteamMessagingMultiplayerPeer::join_lobby);
	ClassDB::bind_method(D_METHOD("activate_invite_dialog"), &SteamMessagingMultiplayerPeer::activate_invite_dialog);
	ClassDB::bind_method(D_METHOD("start_server"), &SteamMessagingMultiplayerPeer::start_server);

	BIND_ENUM_CONSTANT(LobbyPrivacy::OPEN);
	BIND_ENUM_CONSTANT(LobbyPrivacy::FRIENDS);
	BIND_ENUM_CONSTANT(LobbyPrivacy::CLOSED);
}
