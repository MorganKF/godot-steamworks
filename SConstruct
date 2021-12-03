#!/usr/bin/env python
import os
import sys

default_library_name = "GodotSteamworks"
default_target_path = "dist/bin/"

steam_sdk_path = "./sdk/"

cpp_bindings_path = "../godot-cpp/"
godot_headers_path = cpp_bindings_path + "godot-headers/"
cpp_library = "libgodot-cpp"

bits = "64"

# Try to detect the host platform automatically.
# This is used if no `platform` argument is passed
if sys.platform.startswith("linux"):
    host_platform = "linux"
elif sys.platform.startswith("freebsd"):
    host_platform = "freebsd"
elif sys.platform == "darwin":
    host_platform = "osx"
elif sys.platform == "win32" or sys.platform == "msys":
    host_platform = "windows"
else:
    raise ValueError("Could not detect platform automatically, please specify with " "platform=<platform>")

env = Environment(ENV=os.environ)
opts = Variables([], ARGUMENTS)

# Use c++ 17
env.Append(SCONS_CXX_STANDARD="c++17")

# Define our options
opts.Add(EnumVariable("target", "Compilation target", "debug", allowed_values=("debug", "release"), ignorecase=2))
opts.Add(
    EnumVariable(
        "platform",
        "Compilation platform",
        host_platform,
        allowed_values=("linux", "windows", "osx"),
        ignorecase=2,
    )
)
opts.Add(EnumVariable("bits", "Target platform bits", "64", ("32", "64")))
opts.Add(BoolVariable("use_llvm", "Use the LLVM / Clang compiler", "no"))
opts.Add(EnumVariable("macos_arch", "Target macOS architecture", "universal", ["universal", "x86_64", "arm64"]))
opts.Add(PathVariable("target_path", "The path where the lib is installed.", default_target_path, PathVariable.PathAccept))
opts.Add(PathVariable("target_name", "The library name.", default_library_name, PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))

# This makes sure to keep the session environment variables on Windows.
# This way, you can run SCons in a Visual Studio 2017 prompt and it will find
# all the required tools
if host_platform == "windows" and env["platform"] != "android":
    if env["bits"] == "64":
        env = Environment(TARGET_ARCH="amd64")
    elif env["bits"] == "32":
        env = Environment(TARGET_ARCH="x86")

    opts.Update(env)

# Process some arguments
if env["use_llvm"]:
    env["CC"] = "clang"
    env["CXX"] = "clang++"

if env["platform"] == "":
    print("No valid target platform selected.")
    quit()

if env["target"] == "debug":
    env.Append(CPPDEFINES=["DEBUG_ENABLED", "DEBUG_METHODS_ENABLED"])

# Check our platform specifics
if env["platform"] == "osx":
    env["target_path"] += "osx/"
    cpp_library += ".osx"

    # Link steam sdk
    env.Append(LIBS=["steam_api"])
    env.Append(LIBPATH=[steam_sdk_path + "redistributable_bin/osx"])

    if env["bits"] == "32":
        raise ValueError("Only 64-bit builds are supported for the macOS target.")

    if env["macos_arch"] == "universal":
        env.Append(LINKFLAGS=["-arch", "x86_64", "-arch", "arm64"])
        env.Append(CCFLAGS=["-arch", "x86_64", "-arch", "arm64"])
    else:
        env.Append(LINKFLAGS=["-arch", env["macos_arch"]])
        env.Append(CCFLAGS=["-arch", env["macos_arch"]])

    env.Append(CXXFLAGS=["-std=c++17"])
    if env["target"] == "debug":
        env.Append(CCFLAGS=["-g", "-O2"])
    else:
        env.Append(CCFLAGS=["-g", "-O3"])

    arch_suffix = env["macos_arch"]

elif env["platform"] in ("x11", "linux", "server"):
    cpp_library += ".linux"
    env.Append(CCFLAGS=["-fPIC"])

    # Link steam sdk
    env.Append(LIBS=["steam_api"])
    env.Append(LIBPATH=[steam_sdk_path + "redistributable_bin/linux64"])

    env.Append(CXXFLAGS=["-std=c++17"])
    if env["target"] == "debug":
        env.Append(CCFLAGS=["-g3", "-Og"])
    else:
        env.Append(CCFLAGS=["-g", "-O3"])

    arch_suffix = str(bits)
elif env["platform"] == "windows":
    cpp_library += ".windows"
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV=os.environ)

    env.Append(CPPDEFINES=["WIN32", "_WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS"])
    env.Append(CCFLAGS=["-W3", "-GR"])
    env.Append(CXXFLAGS=["-std:c++17"])

    # Link steam sdk
    env.Append(LIBS=["steam_api64"])
    env.Append(LIBPATH=[steam_sdk_path + "redistributable_bin/win64"])

    if env["target"] == "debug":
        env.Append(CPPDEFINES=["_DEBUG"])
        env.Append(CCFLAGS=["-EHsc", "-MDd", "-ZI", "-FS"])
        env.Append(LINKFLAGS=["-DEBUG"])
    else:
        env.Append(CPPDEFINES=["NDEBUG"])
        env.Append(CCFLAGS=["-O2", "-EHsc", "-MD"])

    if not(env["use_llvm"]):
        env.Append(CPPDEFINES=["TYPED_METHOD_BIND"])

    arch_suffix = str(bits)


# suffix our godot-cpp library
cpp_library += "." + env["target"] + "." + arch_suffix

# make sure our binding library is properly includes
env.Append(CPPPATH=[".", godot_headers_path, cpp_bindings_path + "include/", cpp_bindings_path + "gen/include/", steam_sdk_path + "/public"])
env.Append(LIBPATH=[cpp_bindings_path + "bin/"])
env.Append(LIBS=[cpp_library])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

target_name = "{}.{}.{}.{}".format(env["target_name"], env["platform"], env["target"], arch_suffix)
library = env.SharedLibrary(target=env["target_path"] + target_name, source=sources)

Default(library)
