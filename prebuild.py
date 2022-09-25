################################
# LVGL config loader
# @hpsaturn 2022
################################
# Put this file on the root of your PlatformIO LVGL project
# and add the next line on your env board in the platformio.ini file.
#
# extra_scripts = pre:prebuild.py
#
# The lv_conf.h file should be placed in the root lib folder
################################

import os.path
import shutil
from platformio import util
from SCons.Script import DefaultEnvironment

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

# get platformio environment variables
env = DefaultEnvironment()
flavor = env.get("PIOENV")

output_path =  ".pio/libdeps/" + flavor 
os.makedirs(output_path, 0o755, True)
shutil.copy("lib/lv_conf.h", output_path)

