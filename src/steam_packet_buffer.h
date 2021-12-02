#ifndef STEAMWORKS_PACKET_BUFFER_H
#define STEAMWORKS_PACKET_BUFFER_H

#include "core/templates/ring_buffer.h"

template <typename T>
class SteamPacketBuffer {
public:
	SteamPacketBuffer() {
		_packet_buffer.resize(12);
		_current_packet = (uint8_t *)memalloc(_max_packet_size);
	};

	~SteamPacketBuffer() {
		_packet_buffer.resize(0);
		if (_current_packet != nullptr) {
			memfree(_current_packet);
		}
	};

	void get_packet(T **r_buffer, int &r_buffer_size) {
		if (_packets == 0) {
			*r_buffer = nullptr;
			r_buffer_size = 0;
			return;
		}

		_packet_buffer.read((uint8_t *)&r_buffer_size, sizeof(int));

		if (r_buffer_size > _max_packet_size) {
			memfree(_current_packet);
			while (r_buffer_size > _max_packet_size) {
				_max_packet_size = next_power_of_2(_max_packet_size + 1);
			}
			_current_packet = (uint8_t *)memalloc(_max_packet_size);
		}

		_packet_buffer.read((uint8_t *)_current_packet, r_buffer_size);
		_packet_buffer.read((uint8_t *)&_next_from, sizeof(uint32_t));
		_packet_buffer.read((uint8_t *)&_next_to, sizeof(int32_t));
		*r_buffer = _current_packet;
		_packets--;
	};

	void put_packet(uint32_t from, int32_t to, T *p_buffer, int p_buffer_size) {
		while (PROTO_SIZE + p_buffer_size > _packet_buffer.space_left()) {
			_packet_buffer.resize(next_power_of_2(_packet_buffer.size() + 1));
		}

		if (_packet_buffer.data_left() == 0) {
			_next_from = from;
			_next_to = to;
		} else {
			_packet_buffer.write((uint8_t *)&from, sizeof(uint32_t));
			_packet_buffer.write((uint8_t *)&to, sizeof(int32_t));
		}

		_packet_buffer.write((uint8_t *)&p_buffer_size, sizeof(int));
		_packet_buffer.write(p_buffer, p_buffer_size);
		_packets++;
	}

	int get_from() const {
		return _next_from;
	}

	int get_to() const {
		return _next_to;
	}

	bool is_empty() const {
		return _packets == 0;
	}

	int size() const {
		return _packets;
	}

private:
	const int PROTO_SIZE = sizeof(uint32_t) + sizeof(int32_t);
	RingBuffer<uint8_t> _packet_buffer;
	unsigned int _packets = 0;
	uint32_t _next_from = 0;
	int32_t _next_to = 0;
	T *_current_packet;
	unsigned int _max_packet_size = pow(2, 6);
};

#endif