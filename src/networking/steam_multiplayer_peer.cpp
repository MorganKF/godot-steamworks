#include "steam_multiplayer_peer.hpp"

int64_t SteamMultiplayerPeer::_get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) {
    ERR_FAIL_COND_V_MSG(_incoming_packets.empty(), ERR_UNAVAILABLE, "No incoming packets available.");
	if (!_current_packet.message) _current_packet.message->Release();
	_current_packet = _incoming_packets.front();
    _incoming_packets.pop();

    *r_buffer_size = static_cast<int32_t>(_current_packet.message->GetSize()) - HEADER_SIZE;
	*r_buffer = reinterpret_cast<const uint8_t *>(((uint8_t *)_current_packet.message->GetData())[HEADER_SIZE]);
	return OK;
}

int64_t SteamMultiplayerPeer::_put_packet(const uint8_t *p_buffer, int64_t p_buffer_size) {
    ERR_FAIL_COND_V_MSG(!SteamNetworkingMessages(), ERR_UNAVAILABLE, "Steamworks not initialized.");
    ERR_FAIL_COND_V_MSG(!_is_active(), ERR_UNAVAILABLE, "The multiplayer instance isn't currently active.");

	int64_t size = p_buffer_size + HEADER_SIZE;
    uint8_t *packet = _make_packet(DATA, get_unique_id(), _target_peer, p_buffer, p_buffer_size);

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

   if (_is_server()) {
	   if (_target_peer == 1) return OK;
	   else if (_target_peer == 0) {
		   for (auto& peer: _peers) {
			   peer.second->send(_channel, packet, size, flags);
		   }
	   }
	   else if (_target_peer < 0) {
		   auto exclude = -_target_peer;
		   for (auto& peer : _peers) {
			   if (peer.first == exclude) continue;
			   peer.second->send(_channel, packet, size, flags);
		   }
	   } else {
		   _peers.find(_target_peer)->second->send(_channel, packet, size, flags);
	   }
   } else {
	   _peers.find(1)->second->send(_channel, packet, size, flags);
   }

	return OK;
}

uint8_t * SteamMultiplayerPeer::_make_packet(Type p_type, uint32_t p_source, int64_t p_destination, const uint8_t *p_buffer, int64_t p_buffer_size) {
	if (p_buffer_size + HEADER_SIZE > _out_packet_size) {
		::free(_out_packet);
		while (p_buffer_size + HEADER_SIZE > _out_packet_size) {
			_out_packet_size = godot::Math::next_power_of_2(_out_packet_size + 1);
		}
		_out_packet = static_cast<uint8_t *>(Memory::alloc_static(_out_packet_size));
	}
	memcpy(&_out_packet[0], &p_type, sizeof(Type));
	memcpy(&_out_packet[sizeof(Type)], &p_source, sizeof(uint32_t));
	memcpy(&_out_packet[sizeof(Type) + sizeof(int64_t)], &p_destination, sizeof(int64_t));
	memcpy(&_out_packet[HEADER_SIZE], p_buffer, p_buffer_size);
	return _out_packet;
}

int64_t SteamMultiplayerPeer::_poll() {
	ERR_FAIL_COND_V_MSG(!SteamNetworkingMessages(), ERR_UNAVAILABLE, "Steamworks not initialized.");
	ERR_FAIL_COND_V_MSG(!_is_active(), ERR_UNAVAILABLE, "The multiplayer instance isn't currently active.");

	if (_server)
		return _poll_server();
	else
		return _poll_client();
}

int64_t SteamMultiplayerPeer::_poll_client() {
	int num_messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(static_cast<int>(_channel), _messages, MESSAGE_LIMIT);

	for (int i = 0; i < num_messages; i++) {
		Type type;
		int64_t src, dest;
		uint8_t *data = (uint8_t *)_messages[i]->m_pData;
		memcpy(&type, &data, sizeof(Type));
		memcpy(&src, &data[sizeof(Type)], sizeof(int64_t));
		memcpy(&dest, &data[sizeof(Type) + sizeof(int64_t)], sizeof(int64_t));

		switch (type) {
		case DATA: {
			_incoming_packets.push(Packet{
					_messages[i],
					src,
					dest,
			});
		} break;
		}
	}

	return OK;
}

int64_t SteamMultiplayerPeer::_poll_server() {
    int num_messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(static_cast<int>(_channel), _messages, MESSAGE_LIMIT);
	return OK;
}

int64_t SteamMultiplayerPeer::_get_available_packet_count() const {
	return (int64_t)_incoming_packets.size();
}

int64_t SteamMultiplayerPeer::_get_max_packet_size() const {
	return 1 << 28;
}

void SteamMultiplayerPeer::_set_transfer_channel(int64_t p_channel) {
	_channel = p_channel;
}

int64_t SteamMultiplayerPeer::_get_transfer_channel() const {
	return _channel;
}

void SteamMultiplayerPeer::_set_transfer_mode(int64_t p_mode) {
	_transfer_mode = static_cast<TransferMode>(p_mode);
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

SteamMultiplayerPeer::SteamMultiplayerPeer() {
	_out_packet_size = pow(2, 8);
    _out_packet = static_cast<uint8_t *>(Memory::alloc_static(_out_packet_size));
    _messages = static_cast<SteamNetworkingMessage_t **>(Memory::alloc_static(sizeof(SteamNetworkingMessage_t *) * MESSAGE_LIMIT));
}

SteamMultiplayerPeer::~SteamMultiplayerPeer() = default;
