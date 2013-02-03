#!/bin/sh

die()
{
  echo ERROR: $*
  exit 1
}

# Check for whitespace in absolute path; this will cause problems in the
# SpiderMonkey build (https://bugzilla.mozilla.org/show_bug.cgi?id=459089)
# and maybe elsewhere, so we just forbid it
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=`dirname "$SCRIPT"`
case "$SCRIPTPATH" in
  *\ * )
    die "Absolute path contains whitespace, which will break the build - move the game to a path without spaces" ;;
esac

JOBS=${JOBS:="-j2"}

# Some of our makefiles depend on GNU make, so we set some sane defaults if MAKE
# is not set.
case "`uname -s`" in
  "FreeBSD" | "OpenBSD" )
    MAKE=${MAKE:="gmake"}
    ;;
  * )
    MAKE=${MAKE:="make"}
    ;;
esac

# Parse command-line options:

premake_args=""

without_nvtt=false
with_system_nvtt=false
with_system_enet=false
with_system_mozjs185=false
enable_atlas=true

for i in "$@"
do
  case $i in
    --without-nvtt ) without_nvtt=true; premake_args="${premake_args} --without-nvtt" ;;
    --with-system-nvtt ) with_system_nvtt=true; premake_args="${premake_args} --with-system-nvtt" ;;
    --with-system-enet ) with_system_enet=true; premake_args="${premake_args} --with-system-enet" ;;
    --with-system-mozjs185 ) with_system_mozjs185=true; premake_args="${premake_args} --with-system-mozjs185" ;;
    --enable-atlas ) enable_atlas=true ;;
    --disable-atlas ) enable_atlas=false ;;
    -j* ) JOBS=$i ;;
    # Assume any other --options are for Premake
    --* ) premake_args="${premake_args} $i" ;;
  esac
done

premake_args="${premake_args} --collada"
if [ "$enable_atlas" = "true" ]; then
  premake_args="${premake_args} --atlas"
fi

cd "$(dirname $0)"
# Now in build/workspaces/ (where we assume this script resides)

if [ "`uname -s`" = "Darwin" ]; then
  # Set *_CONFIG variables on OS X, to override the path to e.g. sdl-config
  export SDL_CONFIG=${SDL_CONFIG:="$(pwd)/../../libraries/osx/sdl/bin/sdl-config"}
  export WX_CONFIG=${WX_CONFIG:="$(pwd)/../../libraries/osx/wxwidgets/bin/wx-config"}
fi

# Don't want to build bundled libs on OS X
# (build-osx-libs.sh is used instead)
if [ "`uname -s`" != "Darwin" ]; then
  echo "Updating bundled third-party dependencies..."
  echo

  # Build/update bundled external libraries
  (cd ../../libraries/source/fcollada/src && ${MAKE} ${JOBS}) || die "FCollada build failed"
  echo
  if [ "$with_system_mozjs185" = "false" ]; then
    (cd ../../libraries/source/spidermonkey && MAKE=${MAKE} JOBS=${JOBS} ./build.sh) || die "SpiderMonkey build failed"
  fi
  echo
  if [ "$with_system_nvtt" = "false" ] && [ "$without_nvtt" = "false" ]; then
    (cd ../../libraries/source/nvtt && MAKE=${MAKE} JOBS=${JOBS} ./build.sh) || die "NVTT build failed"
  fi
  echo
  if [ "$with_system_enet" = "false" ]; then
    (cd ../../libraries/source/enet && MAKE=${MAKE} JOBS=${JOBS} ./build.sh) || die "ENet build failed"
  fi
  echo
fi

# Now build premake and run it to create the makefiles
cd ../premake/premake4
PREMAKE_BUILD_DIR=build/gmake.unix
# BSD and OS X need different Makefiles
case "`uname -s`" in
  "GNU/kFreeBSD" )
    # use default gmake.unix (needs -ldl as we have a GNU userland and libc)
    ;;
  *"BSD" )
    PREMAKE_BUILD_DIR=build/gmake.bsd
    ;;
  "Darwin" )
    PREMAKE_BUILD_DIR=build/gmake.macosx
    ;;
esac
${MAKE} -C $PREMAKE_BUILD_DIR ${JOBS} || die "Premake build failed"

echo

cd ..

# If we're in bash then make HOSTTYPE available to Premake, for primitive arch-detection
export HOSTTYPE="$HOSTTYPE"

echo "Premake args: ${premake_args}"

premake4/bin/release/premake4 --file="premake4.lua" --outpath="../workspaces/gcc/" ${premake_args} gmake || die "Premake failed"
premake4/bin/release/premake4 --file="premake4.lua" --outpath="../workspaces/codeblocks/" ${premake_args} codeblocks || die "Premake failed"

# Also generate xcode workspaces if on OS X
if [ "`uname -s`" = "Darwin" ]; then
  premake4/bin/release/premake4 --file="premake4.lua" --outpath="../workspaces/xcode3" ${premake_args} xcode3 || die "Premake failed"
  premake4/bin/release/premake4 --file="premake4.lua" --outpath="../workspaces/xcode4" ${premake_args} xcode4 || die "Premake failed"
fi
