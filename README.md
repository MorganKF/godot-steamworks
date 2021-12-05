# Godot Steamworks

⚠️ This extension is a Work In Progress. All features are experimental and undocumented. ⚠️

Godot Steamworks is a Godot 4 extension that provides a convenient wrapper around the steam APIs growing
list of functions and integration with the Godot multiplayer API.

## Building
1. Clone this repository and initialise submodules.
2. Compile godot-cpp:
```
cd godot-cpp
scons target=debug -j 4
cd ..
```
3. Compile the extension:
```
scons target=debug
```
4. Place the corresponding steam distributable and `steam_appid.txt` next to your engine executable.


## Old versions
Branch `3.x` - Contains a Godot module with a "working" multiplayer peer utilizing steam networking messages.


