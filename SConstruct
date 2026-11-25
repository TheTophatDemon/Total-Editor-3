import sys, os, shutil

project_name = "Total Editor 3"

cpp_libs = ""
cpp_tools = []

is_windows = sys.platform.startswith("win")
is_linux = sys.platform.startswith("linux")

if is_linux:
  cpp_libs = 'raylib GL m pthread dl rt X11'
  cpp_tools = ["default", "g++"]
elif is_windows:
  cpp_libs = 'raylib opengl32 gdi32 winmm'
  cpp_tools = ["mingw", "g++"]

env = Environment(
  tools = cpp_tools,
  CPPFLAGS = ['-std=c++17', '-Wall'],
  CPPPATH = ['./libraries/include'],
  LIBPATH = ['./libraries/lib'],
  LIBS = Split(cpp_libs),
)

is_debug = int(ARGUMENTS.get('debug', 0))
if is_debug:
  env.Append(CPPFLAGS=['-g'], CPPDEFINES='DEBUG')
else:
  env.Append(CPPFLAGS=['-O2'])
  
release_folder = 'debug' if is_debug else 'release'

env.Append(OBJPREFIX='../obj/%s/' % release_folder)
sources = []
for root, dirs, files in os.walk("src"):
  sources.append(Glob(f"{root}/*.cpp"))
sources.append(Glob('libraries/src/imgui/*.cpp'))

if is_windows:
  icon_res = env.RES('icon.rc')
  sources.append(icon_res)
  version_res = env.RES('version.rc')
  sources.append(version_res)

bin_dir = os.path.join('bin', release_folder)
env.Program(target=os.path.join(bin_dir, project_name), source=sources)

# Copy assets to release dir
if not is_debug:
  # Scons won't respond to file removals in these directories so we have to clear them manually.
  shutil.rmtree(os.path.join(bin_dir, 'assets'), ignore_errors=True)
  shutil.rmtree(os.path.join(bin_dir, 'instructions'), ignore_errors=True)
  shutil.rmtree(os.path.join(bin_dir, 'LICENSE'), ignore_errors=True)
  env.Install(bin_dir, 'assets')
  env.Install(bin_dir, 'instructions')
  env.Install(bin_dir, 'LICENSE')