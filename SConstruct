import sys

project_name = "Total Editor 3"

env = None
if sys.platform.startswith("linux"):
  env = Environment(
    tools = ['default', 'gcc'],
    CFLAGS = ['-std=c99'],
    CPPPATH = ['./libraries/include'],
    LIBPATH = ['./libraries/lib'],
    CDEFINES = ['LINUX_UBUNTU_64'],
    LIBS = Split('raylib GL m pthread dl rt X11'),
    OBJPREFIX="../obj/"
  )

is_debug = int(ARGUMENTS.get('debug', 0))
if is_debug:
  env.Append(CPPFLAGS=['-g'], CPPDEFINES='DEBUG')
else:
  env.Append(CPPFLAGS=['-O2'])
  
bin_dir = 'debug' if is_debug else 'release'

env.Append(OBJPREFIX='../obj/%s/' % bin_dir)

env.Program(target='./bin/%s/%s' % (bin_dir, project_name), source=Glob('src/*.c'))

#Copy the assets directory into the release directory
Install("./bin/%s/assets" % bin_dir, Glob('assets/**'))
