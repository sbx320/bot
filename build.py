import optparse
import os
import subprocess
import sys
import multiprocessing

parser = optparse.OptionParser()
parser.add_option('--debug',
    action='store_true',
    dest='debug',
    help='build in debug [Without its building release]')

(options, args) = parser.parse_args()

def execute(argv, env=os.environ):
  try:
    subprocess.check_call(argv, env=env)
    return 0
  except subprocess.CalledProcessError as e:
    return e.returncode
  raise e

def RunNinja(configuration):
  env = os.environ.copy()
  var = execute(["ninja","-C", "build/" + configuration], env)
  return var

buildtype = "Debug" if options.debug else "Release"
sys.exit(RunNinja(buildtype))
