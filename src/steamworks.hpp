#ifndef GODOT_STEAMWORKS_STEAMWORKS_H
#define GODOT_STEAMWORKS_STEAMWORKS_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class Steamworks : public RefCounted {
	GDCLASS(Steamworks, RefCounted);

protected:
	static void _bind_methods();

public:
	void init() const;
	void run_callbacks() const;
	Steamworks();
	~Steamworks() override;
};

#endif // GODOT_STEAMWORKS_STEAMWORKS_H
