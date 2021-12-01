#include "steam_networking.h"

SteamMessagingMultiplayerPeer::SteamMessagingMultiplayerPeer() :
		_server(false),
		_target_peer(TARGET_PEER_BROADCAST),
		_transfer_mode(TRANSFER_MODE_RELIABLE),
		_refuse_connections(true),
		_connection_status(CONNECTION_DISCONNECTED),
		_lobby_id(nullptr),
		_peer_id(0) {
	_messages = (SteamNetworkingMessage_t **)memalloc(sizeof(SteamNetworkingMessage_t *) * MESSAGE_LIMIT);
	_out_packet_size = pow(2, 8);
	_out_packet = (uint8_t *)memalloc(_out_packet_size);
}

SteamMessagingMultiplayerPeer::~SteamMessagingMultiplayerPeer() {
	if (_lobby_id != nullptr) {
		memfree(_lobby_id);
	}
	memfree(_messages);
	memfree(_out_packet);
}

/**
 * Opens Steam invite dialog
 */
void SteamMessagingMultiplayerPeer::activate_invite_dialog() {
	if (_lobby_id == nullptr) {
		WARN_PRINT("Attempting to open invite overlay without initializing lobby!")
		return;
	}

	// Todo: Update invites to change for server/lobby
	SteamFriends()->ActivateGameOverlayInviteDialog(*_lobby_id);
}

/**
 * Builds buffer containing internal parameters and Godot data
 */
uint8_t *SteamMessagingMultiplayerPeer::make_network_packet(PacketType p_type, uint32_t p_source, int32_t p_destination, const uint8_t *p_buffer, int p_buffer_size) {
	if (p_buffer_size + PROTO_SIZE > _out_packet_size) {
		memfree(_out_packet);
		while (p_buffer_size + PROTO_SIZE > _out_packet_size) {
			_out_packet_size = next_power_of_2(_out_packet_size + 1);
		}
		_out_packet = (uint8_t *)memalloc(_out_packet_size);
	}
	memcpy(&_out_packet[0], &p_type, sizeof(PacketType));
	memcpy(&_out_packet[sizeof(PacketType)], &p_source, sizeof(uint32_t));
	memcpy(&_out_packet[sizeof(PacketType) + sizeof(uint32_t)], &p_destination, sizeof(int32_t));
	memcpy(&_out_packet[PROTO_SIZE], p_buffer, p_buffer_size);
	return _out_packet;
}

/**
 * Creates a steam matchmaking lobby
 */
void SteamMessagingMultiplayerPeer::create_lobby(LobbyPrivacy p_lobby_type, int p_max_players) {
	if (SteamMatchmaking() == nullptr) {
		ERR_PRINT("SteamAPI has not been initialized!")
		return;
	}

	_connection_status = CONNECTION_CONNECTING;
	const SteamAPICall_t api_call = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, p_max_players);
	m_lobby_created_call_result.Set(api_call, this, &SteamMessagingMultiplayerPeer::on_lobby_created);
}

/**
 * Sets lobby game server to host user
 * Todo: Allow servers to be started without first creating a lobby
 * Todo: Only allow / server start from owning player
 */
void SteamMessagingMultiplayerPeer::start_server() {
	_peer_id = 1;
	_refuse_connections = false;
	_connection_status = CONNECTION_CONNECTED;
	SteamMatchmaking()->SetLobbyGameServer(*_lobby_id, 0, 0, SteamUser()->GetSteamID());
}

/**
 * Joins a lobby
 * Todo: Reset connection information to allow joining a lobby while connected to / hosting a server
 * without recreating the object
 */
Error SteamMessagingMultiplayerPeer::join_lobby(uint64_t p_game_id) {
	if (SteamMatchmaking() == nullptr) {
		ERR_PRINT("SteamAPI has not been initialized!")
		return ERR_CANT_ACQUIRE_RESOURCE;
	}
	_connection_status = CONNECTION_CONNECTING;
	SteamMatchmaking()->JoinLobby(CSteamID(p_game_id));
	return OK;
}

/**
 * Called when a lobby is created
 * Todo: Allow server host to be migrated to other lobby members
 */
void SteamMessagingMultiplayerPeer::on_lobby_created(LobbyCreated_t *p_callback, bool p_io_failure) {
	if (p_callback->m_eResult == k_EResultOK) {
		if (_lobby_id == nullptr) {
			_lobby_id = (CSteamID *)memalloc(sizeof(CSteamID));
		}
		*_lobby_id = CSteamID(p_callback->m_ulSteamIDLobby);
		_server = true;
	} else {
		// Todo
	}
}

/**
 * Called when Godot wants a packet
 */
Error SteamMessagingMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	_packet_buffer.get_packet((uint8_t **)r_buffer, r_buffer_size);
	return OK;
}

/**
 * Called when Godot wants to send a packet to peers
 * Todo: Do something with SendMessageToUser's return
 */
Error SteamMessagingMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	if (SteamNetworking() == nullptr) {
		return ERR_UNAVAILABLE;
	}

	auto size = p_buffer_size + PROTO_SIZE;
	uint8_t *packet = make_network_packet(DATA, get_unique_id(), _target_peer, p_buffer, p_buffer_size);

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
			return OK; // Don't send to self
		} else if (_target_peer == 0) {
			// Send to everyone
			for (auto element = _peer_map.front(); element; element = element->next()) {
				SteamNetworkingMessages()->SendMessageToUser(element->value(), packet, size, flags, CHANNEL);
			}
		} else if (_target_peer < 0) {
			// Send to all excluding one
			// Todo: We should probably exclude the server
			for (auto element = _peer_map.front(); element; element = element->next()) {
				if (element->key() != -_target_peer) {
					SteamNetworkingMessages()->SendMessageToUser(element->value(), packet, size, flags, CHANNEL);
				}
			}
		} else {
			// Send to target
			SteamNetworkingMessages()->SendMessageToUser(_peer_map[_target_peer], packet, size, flags, CHANNEL);
		}
	} else {
		// We are a client so messages can only go to the server
		const auto result = SteamNetworkingMessages()->SendMessageToUser(_peer_map[1], packet, size, flags, CHANNEL);

		if (result == k_EResultNoConnection) {
			return ERR_CANT_CONNECT;
		}
	}

	return OK;
}

/**
 * Receives packets from the steam messages system and places them into a buffer for future use.
 * Todo: Do something with SendMessageToUser's return
 */
void SteamMessagingMultiplayerPeer::poll() {
	// Get latest network messages
	int num_messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(CHANNEL, _messages, MESSAGE_LIMIT);

	for (auto i = 0; i < num_messages; i++) {
		// Unpack message
		PacketType type;
		int src, dest;
		int size = _messages[i]->m_cbSize - PROTO_SIZE;
		uint8_t *data = (uint8_t *)_messages[i]->m_pData;
		memcpy(&type, &data[0], sizeof(PacketType));
		memcpy(&src, &data[sizeof(PacketType)], sizeof(uint32_t));
		memcpy(&dest, &data[sizeof(PacketType) + sizeof(uint32_t)], sizeof(int32_t));

		switch (type) {
			case PacketType::DATA: {
				_packet_buffer.put_packet(src, dest, &data[PROTO_SIZE], size);
			} break;

			// Used to set client unique ids
			case PacketType::SYS_SET_ID: {
				if (src == 1) {
					emit_signal("connection_succeeded");
					emit_signal("peer_connected", dest);
					_peer_id = dest;
					_connection_status = CONNECTION_CONNECTED;
				}
			} break;

			// Initializes internal data for server peers
			case PacketType::SYS_INIT: {
				if (_server) {
					// Generate ID
					int id;
					if (_peer_map.size() == 0) {
						id = 2;
					} else {
						id = _peer_map.back()->key() + 1;
					}
					_peer_map[id] = _messages[i]->m_identityPeer;

					// Emit Godot signals
					emit_signal("peer_connected", id);
					emit_signal("connected_to_server");

					// Send ID to new peer
					auto out_packet = make_network_packet(SYS_SET_ID, _peer_id, id, nullptr, 0);
					SteamNetworkingMessages()->SendMessageToUser(_messages[i]->m_identityPeer, out_packet, PROTO_SIZE, k_nSteamNetworkingSend_Reliable, CHANNEL);
				} else {
					WARN_PRINT("GOT SIS_INIT FROM PLAYER?")
				}
			} break;
		}

		// Free steam data
		_messages[i]->Release();
	}
}

/**
 * Called when current client enters the lobby
 * Todo: Reset connection data when joining to a new lobby
 */
void SteamMessagingMultiplayerPeer::on_lobby_enter(LobbyEnter_t *p_callback) {
	if (!_server) {
		if (_lobby_id == nullptr) {
			_lobby_id = (CSteamID *)memalloc(sizeof(CSteamID));
		}
		*_lobby_id = CSteamID(p_callback->m_ulSteamIDLobby);
	}
	emit_signal("lobby_joined");
}

/**
 * Called when lobby metadata is updated
 * Todo: Expose methods for manipulating lobby metadata
 */
void SteamMessagingMultiplayerPeer::on_lobby_updated(LobbyDataUpdate_t *p_callback) {
	emit_signal("lobby_updated");
}

/**
 * Called when peer requests a connection
 * Todo: Only accept connections on hosts
 * Todo: Limit connections to max players
 * Todo: Simple security checks?
 */
void SteamMessagingMultiplayerPeer::on_session_request(SteamNetworkingMessagesSessionRequest_t *p_callback) {
	if (!_refuse_connections) {
		SteamNetworkingMessages()->AcceptSessionWithUser(p_callback->m_identityRemote);
	}
}

/**
 * Called when a server has been set for the lobby
 * Todo: Handle errors / unable to connect
 */
void SteamMessagingMultiplayerPeer::on_game_created(LobbyGameCreated_t *p_callback) {
	// Todo: check if steam id is actually set
	if (!_server) {
		auto packet = make_network_packet(SYS_INIT, 0, 1, nullptr, 0);
		auto id = SteamNetworkingIdentity();
		id.SetSteamID64(p_callback->m_ulSteamIDGameServer);
		_peer_map[1] = id;
		SteamNetworkingMessages()->SendMessageToUser(id, packet, PROTO_SIZE, k_nSteamNetworkingSend_Reliable, CHANNEL);
	}
}

void SteamMessagingMultiplayerPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_lobby", "lobby_type", "max_players"), &SteamMessagingMultiplayerPeer::create_lobby);
	ClassDB::bind_method(D_METHOD("join_lobby"), &SteamMessagingMultiplayerPeer::join_lobby);
	ClassDB::bind_method(D_METHOD("activate_invite_dialog"), &SteamMessagingMultiplayerPeer::activate_invite_dialog);
	ClassDB::bind_method(D_METHOD("start_server"), &SteamMessagingMultiplayerPeer::start_server);

	ADD_SIGNAL(MethodInfo("lobby_joined"));
	ADD_SIGNAL(MethodInfo("lobby_updated"));
}

/////////////////////////
/// Getters / Setters ///
/////////////////////////

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
	ERR_FAIL_COND_V(_packet_buffer.is_empty(), 1)
	_packet_buffer.get_from();
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
	return _packet_buffer.size();
}

int SteamMessagingMultiplayerPeer::get_max_packet_size() const {
	return std::numeric_limits<int>::max() - PROTO_SIZE;
}