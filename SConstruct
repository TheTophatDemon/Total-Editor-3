import sys, os

project_name = "Total Editor 3"

cpp_platform = ""
cpp_libs = ""
cpp_tools = []

if sys.platform.startswith("linux"):
    cpp_platform = "LINUX_64"
    cpp_libs = 'raylib GL m pthread dl rt X11'
    cpp_tools = ["g++"]
elif sys.platform.startswith("win"):
    cpp_platform = "WINDOWS_64"
    cpp_libs = 'raylib opengl32 gdi32 winmm'
    cpp_tools = ["mingw", "g++"]

env = Environment(
  tools = cpp_tools,
  CPPFLAGS = ['-std=c++17'],
  CPPPATH = ['./libraries/include'],
  LIBPATH = ['./libraries/lib'],
  OBJPREFIX="../obj/",
  CPPDEFINES = [cpp_platform],
  LIBS = Split(cpp_libs),
)

is_debug = int(ARGUMENTS.get('debug', 0))
if is_debug:
  env.Append(CPPFLAGS=['-g'], CPPDEFINES='DEBUG')
else:
  env.Append(CPPFLAGS=['-O2'])
  
bin_dir = 'debug' if is_debug else 'release'

env.Append(OBJPREFIX='../obj/%s/' % bin_dir)
sources = []
sources.append(Glob('src/*.cpp'))
sources.append(Glob('libraries/src/imgui/*.cpp'))
env.Program(target='./bin/%s/%s' % (bin_dir, project_name), source=sources)
