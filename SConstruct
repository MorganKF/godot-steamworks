#!/usr/bin/env python
import os
import sys

env = SConscript("./godot-cpp/SConstruct")

env.Append(COMPILATIONDB_USE_ABSPATH=True)
env.Tool('compilation_db')
Alias('compiledb', env.CompilationDatabase('compile_commands.json'))

if env["platform"] == "osx":
    env.Append(LIBS=["steam_api"])
    env.Append(LIBPATH=["sdk/redistributable_bin/osx"])

elif env["platform"] in ("x11", "linux", "server"):
    env.Append(LIBS=["steam_api"])
    env.Append(LIBPATH=["sdk/redistributable_bin/linux64"])
elif env["platform"] == "windows":
    env.Append(LIBS=["steam_api64"])
    env.Append(LIBPATH=["sdk/redistributable_bin/win64"])

env.Append(CPPPATH=["sdk/public"])
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources += Glob("src/*/*.cpp")

if env["platform"] == "osx":
    library = env.SharedLibrary(
        "dist/bin/godot_steamworks.{}.{}.framework/godot_steamworks.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "dist/bin/godot_steamworks.{}.{}.{}{}".format(
            env["platform"], env["target"], env["arch_suffix"], env["SHLIBSUFFIX"]
        ),
        source=sources,
    )

Default(library)
