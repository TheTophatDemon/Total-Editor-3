import sys

project_name = "Total Editor 3"

env = Environment(
  tools = ['default'],
  CPPFLAGS = ['-std=c++17'],
  CPPPATH = ['./libraries/include'],
  LIBPATH = ['./libraries/lib'],
  OBJPREFIX="../obj/"
)

if sys.platform.startswith("linux"):
  env.Append(tools = ['g++'])
  env.Append(CPPDEFINES = ['LINUX_64'])
  env.Append(LIBS = Split('raylib GL m pthread dl rt X11'))
elif sys.platform.startswith("windows"):
  #TODO: Test to see if this compiles on Windows
  env.Append(tools = ['mingw']) #TODO: Also consider supporting VC++ somehow
  env.Append(CPPDEFINES = ['WINDOWS_64'])
  env.Append(LIBS = Split('raylib opengl32 gdi32 winmm'))

is_debug = int(ARGUMENTS.get('debug', 0))
if is_debug:
  env.Append(CPPFLAGS=['-g'], CPPDEFINES='DEBUG')
else:
  env.Append(CPPFLAGS=['-O2'])
  
bin_dir = 'debug' if is_debug else 'release'

env.Append(OBJPREFIX='../obj/%s/' % bin_dir)

env.Program(target='./bin/%s/%s' % (bin_dir, project_name), source=Glob('src/*.cpp'))
