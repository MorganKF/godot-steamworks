# config.py

def can_build(env, platform):
    return True

def configure(env):
	env.Append(CPPPATH=["#modules/godot_steamworks/sdk/public/"])

	if env["platform"]== "x11" or env["platform"] == "server":
		env.Append(LIBS=["steam_api"])
		env.Append(RPATH=["."])
		if env["bits"]=="32":
			env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin/linux32"])
		else:
			env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin/linux64"])

	elif env["platform"] == "windows":
		if env["CC"] == "cl":
			if env["bits"]=="32":
				env.Append(LINKFLAGS=["steam_api.lib"])
				env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin"])
			else:
				env.Append(LINKFLAGS=["steam_api64.lib"])
				env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin/win64"])
		
		else:
			if env["bits"]=="32":
				env.Append(LIBS=["steam_api"])
				env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin"])
			else:
				env.Append(LIBS=["steam_api64"])
				env.Append(LIBPATH=["#modules/godot_steamworks/sdk/redistributable_bin/win64"])

	elif env["platform"] == "osx":
		env.Append(LIBS=["steam_api"])
		env.Append(LIBPATH=['#modules/godot_steamworks/sdk/redistributable_bin/osx'])

def get_doc_classes():
	return [
		"Steamworks",
	]

def get_doc_path():
	return "doc_classes"
