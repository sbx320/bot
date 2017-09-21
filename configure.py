#!/usr/bin/env python

import optparse
import os
import pprint
import re
import shlex
import subprocess
import sys
root_dir = os.path.dirname(__file__)
sys.path.insert(0, os.path.join(root_dir, 'tools', 'gyp', 'pylib'))
sys.path.insert(0, os.path.abspath(os.path.join(root_dir, 'tools')))
import shutil
import string
import glob
import shlex
import sys
import platform
import gyp

from gyp.common import GetFlavor

# parse our options
parser = optparse.OptionParser()

parser.add_option('--ninja',
    action='store_true',
    dest='ninja',
    help='use ninja to build')

(options, args) = parser.parse_args()

gyp_args = ['--no-parallel']

flavor = GetFlavor({})
if flavor == 'win' and not options.ninja:
  gyp_args += ['-f', 'msvs', '-G', 'msvs_version=auto']
else:
  gyp_args += ['-f', 'ninja']

def run_gyp(args):
  rc = gyp.main(args)
  if rc != 0:
    print('Error running GYP')
    sys.exit(rc)

if __name__ == '__main__':
  args = gyp_args

  # GYP bug.
  if sys.platform == 'win32':
    args.append(os.path.join(os.path.dirname(__file__), 'rd2lbot.gyp'))
    toolchain_fn  = os.path.join(os.path.dirname(__file__), 'toolchain.gypi')
  else:
    args.append(os.path.join(os.path.dirname(__file__), 'rd2lbot.gyp'))
    toolchain_fn  = os.path.join(os.path.dirname(__file__), 'toolchain.gypi')

  if os.path.exists(toolchain_fn):
    args.extend(['-I', toolchain_fn])

  args.append('--depth=.')
  #args.append('-Dcomponent=shared_library')
  #args.append('-Dlibrary=shared_library')
  gyp_args = list(args)
  
  gyp_args.append('--generator-output=build')
  gyp_args.append('-Goutput_dir=.')
  
  run_gyp(gyp_args)
