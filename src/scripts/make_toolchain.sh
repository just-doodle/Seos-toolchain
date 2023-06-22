#!/bin/bash

set -e

SCRIPTDIR="$PWD/src/scripts/"
SYSROOT="$HOME/sysroot"
TESTDIR="$SYSROOT/installed"
I686_ELF_TOOLCHAIN_ZIP="$SYSROOT/src/build-i686-elf/i686-elf-tools-linux.zip"
PATCHDIR="$PWD/patches"

GCC_VERSION=12.2.0
BINUTILS_VERSION=2.40

PACKAGE_PATH=0
export PARALLEL=false
export HELP=false
export CLEAN_ALL=false
export CLEAN_NEWLIB=false
export CLEAN_I686_ELF_TOOLS=false
export NEWLIB=false
export OSTOOLS=false
export I686_ELF_TOOLS=false

export BUILD_ALL=true

export USE_DEFAULT=false
export ADD_PATH=false

export DOWNLOAD_I686_ELF_TOOLS=false

function backup_path
{
    echo "$PATH" > "$TESTDIR/PATH_${1}"
}

function restore_path
{
    read -r line < "$TESTDIR/PATH_${1}"
    export PATH="$line"
}

function add_path
{
    export PATH="$1":$PATH
}

function extract_i686_elf
{
    if ls "$TESTDIR"/I686_ELF_TOOLCHAIN 1> /dev/null 2>&1; then 
        sleep 0
    else
        mkdir -p "$SYSROOT/installed"
        unzip "$I686_ELF_TOOLCHAIN_ZIP" -d "$SYSROOT"
        touch "$TESTDIR"/I686_ELF_TOOLCHAIN
    fi
}

function build_i686_elf
{
    echo "Building i686-elf toolset"
    if ls "$TESTDIR/I686_ELF_TOOLS" 1> /dev/null 2>&1; then
        echo "I686_elf toolchain is already built. To reinstall, remove $TESTDIR/I686_ELF_TOOLS"
    else
        if [[ $PARALLEL == true ]]; then
            "$SCRIPTDIR"i686-elf-tools.sh linux gcc binutils zip -parallel --build_directory "$SYSROOT/src/build-i686-elf"
        else
            "$SCRIPTDIR"i686-elf-tools.sh linux gcc binutils zip --build_directory "$SYSROOT/src/build-i686-elf"
        fi
        touch "$TESTDIR/I686_ELF_TOOLS"
        extract_i686_elf
    fi
}

function download_i686_elf
{
    if ls "$TESTDIR/I686_ELF_TOOLS" 1> /dev/null 2>&1; then
        echo "I686_elf toolchain is already built. To reinstall, remove $TESTDIR/I686_ELF_TOOLS"
    else
        mkdir -p "$SYSROOT/src/build-i686-elf/"
        wget https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/i686-elf-tools-linux.zip  -O "$I686_ELF_TOOLCHAIN_ZIP"
        touch "$TESTDIR/I686_ELF_TOOLS"
        extract_i686_elf
    fi
}

function build_autoconf_2_69
{
    if ls "$TESTDIR/AUTOCONF_2.69" 1> /dev/null 2>&1; then
        echo "Autoconf 2.69 is already built. To rebuild it, remove $TESTDIR/AUTOCONF_2.69"
    else
        if ls "$SYSROOT/src/autoconf-2.69.tar.gz" 1> /dev/null 2>&1; then
            sleep 0
        else
            wget https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz -O "$SYSROOT/src/autoconf-2.69.tar.gz"
        fi
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/autoconf-2.69.tar.gz"
        mkdir -p "$SYSROOT/src/build-autoconf"
        mkdir -p "$SYSROOT/autotools-old"
        pushd "$SYSROOT/src/build-autoconf"
        ../autoconf-2.69/configure --prefix="${SYSROOT}"/autotools-old
        if [[ $PARALLEL == true  ]]; then
            make -j4
            make install -j4
        else
            make
            make install
        fi
        touch "$TESTDIR/AUTOCONF_2.69"
        popd
        popd
    fi
}

function build_automake_1_11()
{
    if ls "$TESTDIR/AUTOMAKE_1.11" 1> /dev/null 2>&1; then
        echo "Automake 1.11 is already built. To rebuild it, remove $TESTDIR/AUTOMAKE_1.11"
    else
        if ls "$SYSROOT/src/automake-1.11.tar.gz" 1> /dev/null 2>&1; then
            sleep 0
        else
            wget https://ftp.gnu.org/gnu/automake/automake-1.11.tar.gz -O "$SYSROOT/src/automake-1.11.tar.gz"
        fi
        
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/automake-1.11.tar.gz"
        mkdir -p "$SYSROOT/src/build-automake"
        mkdir -p "$SYSROOT/autotools-old"
        pushd "$SYSROOT/src/build-automake"
        ../automake-1.11/configure --prefix="$SYSROOT/autotools-old"
        if [[ $PARALLEL == true ]]; then
            make -j4
            make install -j4
        else
            make
            make install
        fi
        touch "$TESTDIR/AUTOMAKE_1.11"
        popd
        popd
    fi
}

function build_automake_1_15_1()
{
    if ls "$TESTDIR/AUTOMAKE_1.15.1" 1> /dev/null 2>&1; then
        echo "Automake 1.15.1 is already built. To rebuild it, remove $TESTDIR/AUTOMAKE_1.15.1"
    else
        if ls "$SYSROOT/src/automake-1.15.1.tar.gz" 1> /dev/null 2>&1; then
            sleep 0
        else
            wget https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz -O "$SYSROOT/src/automake-1.15.1.tar.gz"
        fi
        
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/automake-1.15.1.tar.gz"
        mkdir -p "$SYSROOT/src/build-automake-new"
        mkdir -p "$SYSROOT/autotools-old"
        pushd "$SYSROOT/src/build-automake-new"
        ../automake-1.15.1/configure --prefix="$SYSROOT/autotools-old"
        if [[ $PARALLEL == true  ]]; then
            make -j4
            make install -j4
        else
            make
            make install
        fi
        touch "$TESTDIR/AUTOMAKE_1.15.1"
        popd
        popd
    fi
}

function newlib_prerequsites
{
    backup_path autoold
    build_autoconf_2_69
    build_automake_1_11
    add_path "$SYSROOT/autotools-old/bin"
}

function download_newlib
{
    if ls "$SYSROOT/src/newlib.tar.gz" 1> /dev/null 2>&1; then
        printf ""
    else
        wget ftp://sourceware.org/pub/newlib/newlib-2.5.0.20171222.tar.gz -O "$SYSROOT/src/newlib.tar.gz"
    fi
}

function extract_newlib
{
    if ls "$SYSROOT/src/newlib" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/newlib.tar.gz"
        mv "$SYSROOT/src/newlib-2.5.0.20171222/" "$SYSROOT/src/newlib/"
        popd
    fi
}

function patch_newlib
{
    if ls "$TESTDIR/NEWLIB_PATCHED" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/newlib/"
        patch -p0 < "$PATCHDIR/newlib/newlib.patch"
        cp -r "$PATCHDIR/newlib/sectoros" newlib/libc/sys/
        popd
        touch "$TESTDIR/NEWLIB_PATCHED"
    fi
}

function build_newlib
{
    if ls "$TESTDIR/NEWLIB" 1> /dev/null 2>&1; then
        echo "Newlib is already built. To rebuild it, remove $TESTDIR/NEWLIB"
    else
        download_newlib
        extract_newlib
        patch_newlib
        newlib_prerequsites

        pushd "$SYSROOT/src/newlib/newlib"
        pushd "libc/sys"
        autoconf
        popd
        pushd "libc/sys/sectoros"
        autoreconf
        popd
        popd

        mkdir -p "$SYSROOT/fakehost"
        ln "$SYSROOT/bin/i686-elf-ar" "$SYSROOT/fakehost/i686-sectoros-ar"
        ln "$SYSROOT/bin/i686-elf-as" "$SYSROOT/fakehost/i686-sectoros-as"
        ln "$SYSROOT/bin/i686-elf-gcc" "$SYSROOT/fakehost/i686-sectoros-gcc"
        ln "$SYSROOT/bin/i686-elf-gcc" "$SYSROOT/fakehost/i686-sectoros-cc"
        ln "$SYSROOT/bin/i686-elf-ranlib" "$SYSROOT/fakehost/i686-sectoros-ranlib"
        add_path "$SYSROOT/fakehost/"

        mkdir -p "$SYSROOT/src/build_newlib"
        pushd "$SYSROOT/src/build_newlib"
        ../newlib/configure --prefix=/usr --target=i686-sectoros
        if [[ $PARALLEL == true ]]; then
            make all -j4
            make DESTDIR=${SYSROOT} install -j4
        else
            make all
            make DESTDIR=${SYSROOT} install
        fi
        popd
        cp -ar $SYSROOT/usr/i686-sectoros/* $SYSROOT/usr/
        cp "$PATCHDIR/../src/include/user_syscall.h" $SYSROOT/usr/include/
        touch "$TESTDIR/NEWLIB"
        restore_path autoold
        add_path "$SYSROOT/bin"
        rm -rf "$SYSROOT/fakehost" || true
    fi
}

function download_gcc
{
    if ls "$SYSROOT/src/gcc-$GCC_VERSION.tar.xz" 1> /dev/null 2>&1; then
        sleep 0
    else
        wget http://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz -O "$SYSROOT/src/gcc-$GCC_VERSION.tar.xz"
    fi
}

function extract_gcc
{
    if ls "$SYSROOT/src/gcc-$GCC_VERSION" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/gcc-$GCC_VERSION.tar.xz"
        popd
    fi
}

function patch_gcc
{
    if ls "$TESTDIR/GCC_PATCHED" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/gcc-$GCC_VERSION"
        patch -p0 < "$PATCHDIR/gcc/gcc.patch"
        cp "$PATCHDIR/gcc/gcc/config/sectoros.h" "$SYSROOT/src/gcc-$GCC_VERSION/gcc/config/"
        touch "$TESTDIR/GCC_PATCHED"
        popd
    fi
}

function build_os_gcc
{
    if ls "$TESTDIR/GCC" 1> /dev/null 2>&1; then
        echo "gcc $GCC_VERSION is already built. To rebuild it, remove $TESTDIR/GCC"
    else
        download_gcc
        extract_gcc
        patch_gcc

        build_automake_1_15_1
        build_autoconf_2_69

        backup_path "autonew"
        rm "$TESTDIR/AUTOMAKE_1.11" || true
        add_path "$SYSROOT/autotools-old/bin"

        pushd "$SYSROOT/src/gcc-$GCC_VERSION"
        pushd libstdc++-v3/
        autoconf
        popd
        popd

        mkdir "$SYSROOT/src/build_gcc"
        pushd "$SYSROOT/src/build_gcc"
        ../gcc-$GCC_VERSION/configure --target=i686-sectoros --prefix="$SYSROOT/usr" --with-sysroot=$SYSROOT --enable-languages=c,c++
        
        if [[ $PARALLEL == true ]]; then
            make all-gcc all-target-libgcc -j4
            make install-gcc install-target-libgcc -j4
            make all-target-libstdc++-v3 -j4
            make install-target-libstdc++-v3 -j4
        else
            make all-gcc all-target-libgcc
            make install-gcc install-target-libgcc
            make all-target-libstdc++-v3
            make install-target-libstdc++-v3
        fi

        cp -ar $SYSROOT/usr/i686-sectoros/* $SYSROOT/usr/
        restore_path "autonew"

        popd
        touch "$TESTDIR/GCC"
    fi
}

function download_binutils
{
    if ls "$SYSROOT/src/binutils-$BINUTILS_VERSION.tar.xz" 1> /dev/null 2>&1; then
        sleep 0
    else
        wget http://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz -O "$SYSROOT/src/binutils-$BINUTILS_VERSION.tar.xz"
    fi
}

function extract_binutils
{
    if ls "$SYSROOT/src/binutils-$BINUTILS_VERSION" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/"
        tar -xf "$SYSROOT/src/binutils-$BINUTILS_VERSION.tar.xz"
        popd
    fi
}

function patch_binutils
{
    if ls "$TESTDIR/BINUTILS_PATCHED" 1> /dev/null 2>&1; then
        sleep 0
    else
        pushd "$SYSROOT/src/binutils-$BINUTILS_VERSION"
        patch -p0 < "$PATCHDIR/binutils/binutils.patch"
        cp "$PATCHDIR"/binutils/emulparams/* ld/emulparams/
        touch "$TESTDIR/BINUTILS_PATCHED"
        popd
    fi
}

function build_os_binutils
{
    if ls "$TESTDIR/BINUTILS" 1> /dev/null 2>&1; then
        echo "Binutils $BINUTILS_VERSION is already built. To rebuild it, remove $TESTDIR/BINUTILS"
    else
        download_binutils
        extract_binutils
        patch_binutils

        build_automake_1_15_1
        build_autoconf_2_69

        backup_path "autonew"
        rm "$TESTDIR/AUTOMAKE_1.11" || true
        add_path "$SYSROOT/autotools-old/bin"

        pushd "$SYSROOT/src/binutils-$BINUTILS_VERSION"
        pushd ld/
        automake
        popd
        popd

        mkdir "$SYSROOT/src/build_binutils"
        pushd "$SYSROOT/src/build_binutils"
        ../binutils-$BINUTILS_VERSION/configure --target=i686-sectoros --prefix="$SYSROOT/usr" --with-sysroot=$SYSROOT --disable-werror
        
        if [[ $PARALLEL == true ]]; then
            make -j4
            make install -j4
        else
            make
            make install
        fi

        restore_path "autonew"
        popd
        touch "$TESTDIR/BINUTILS"
    fi
}

function clean_newlib
{
    rm -rf "$SYSROOT/src/newlib" "$SYSROOT/src/newlib.tar.gz" "$SYSROOT/src/build_newlib" "$SYSROOT/src/autoconf-2.69" "$SYSROOT/src/autoconf-2.69.tar.gz" "$SYSROOT/src/automake-1.11" "$SYSROOT/src/automake-1.11.tar.gz" "$SYSROOT/src/build-autotools" "$TESTDIR/AUTOCONF_2.69" "$TESTDIR/AUTOMAKE_1.11" "$TESTDIR/NEWLIB" "$TESTDIR/NEWLIB_PATCHED" "$SYSROOT/fakehost"
}

function install_to_bashrc
{
    if ls ~/.sectoros 1> /dev/null 2>&1; then
        echo "Executable path is already added to PATH variable. To add it, remove ~/.sectoros"
    else
        cp ~/.bashrc ~/.bashrc_backup
        if [[ $USE_DEFAULT == true ]]; then
            echo "# Added by $(basename "${BASH_SOURCE[0]}") on $(date)" >> ~/.bashrc
            echo "export PATH=$SYSROOT/bin:$SYSROOT/usr/bin:$SYSROOT/libexec:$SYSROOT/usr/libexec:\$PATH" >> ~/.bashrc
        else
            echo "# Added by $(basename "${BASH_SOURCE[0]}") on $(date)" >> ~/.bashrc
            echo "export PATH=\$PATH:$SYSROOT/bin:$SYSROOT/usr/bin:$SYSROOT/libexec:$SYSROOT/usr/libexec" >> ~/.bashrc
        fi
        touch ~/.sectoros
        echo "Executable paths added to PATH variable. The backup of old bashrc is ~/.bashrc_backup"
    fi
}

function clean_ostools
{
    rm -rf "$SYSROOT/../build-i686-elf" "$TESTDIR/I686_ELF_TOOLS"
}

function package
{
    pushd "$SYSROOT"
    tar -cf "$1" bin i686-elf lib libexec share usr
}

while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -s  | --sysroot) SYSROOT="$2";                                     shift ; shift;;
        --parallel_build) PARALLEL=true;                                   shift ;;
        -ca | --clean_all) CLEAN_ALL=true;                                 shift ;;
        -cn | --clean_newlib) CLEAN_NEWLIB=true;                           shift ;;
        -co | --clean_ostools) CLEAN_I686_ELF_TOOLS=true;                  shift ;;
        newlib) NEWLIB=true;               BUILD_ALL=false;                shift ;;
        ostools) OSTOOLS=true;             BUILD_ALL=false;                shift ;;
        i686_elf_toolchain) I686_ELF_TOOLS=true;   BUILD_ALL=false;        shift ;;
        download_i686_elf_tools) DOWNLOAD_I686_ELF_TOOLS=true;             shift ;;
        --add_path) ADD_PATH=true;                                         shift ;;
        --use_as_default) USE_DEFAULT=true;                                shift ;;
        --package) PACKAGE_PATH="$2";                                      shift ;;
        -h |--help) HELP=true;                                             shift ;;
        *)                                                                 shift ;;
    esac
done

if [[ $HELP == true ]]; then
    echo "$0 [options]"
    echo ""
    echo "A script to build toolchain to build the sectoros"
    echo ""
    echo "options:"
    echo "newlib: Enable building of newlib. Newlib is used as the standard library for the sectoros kernel"
    echo "i686_elf_toolchain: Enable building of i686 elf toolchain. Used for building the Sectoros kernel"
    echo "ostools: Enable building of toolchain for making programs. [Builds gcc and binutils of target i686-sectoros]"
    echo "--parallel_build: To allow parallel builds. Adds -j4 to all make instances"
    echo "-ca | --clean_all: Cleans the build directory and src directory"
    echo "-cn | --clean_newlib: Cleans the newlib directory and its files"
    echo "-co | --clean_ostools: Cleans the ostools [i686-elf] directory"
    echo "-h  | --help: Prints this message"
    exit 0
fi

if [[ $CLEAN_ALL == true  ]]; then
    clean_newlib
    clean_ostools
fi

if [[ $CLEAN_NEWLIB == true  ]]; then
    clean_newlib
fi

if [[ $CLEAN_I686_ELF_TOOLS == true ]]; then
    clean_ostools
fi

TESTDIR="$SYSROOT/installed"
I686_ELF_TOOLCHAIN_ZIP="$SYSROOT/src/build-i686-elf/i686-elf-tools-linux.zip"


if [[ $BUILD_ALL == true ]]; then
    NEWLIB=true;
    I686_ELF_TOOLS=true;
    OSTOOLS=true;
fi

echo "SYSROOT          = ${SYSROOT}"
echo "BUILD NEWLIB     = ${NEWLIB}"
echo "BUILD_OSTOOLS    = ${OSTOOLS}"
echo "BUILD_I686_ELF   = ${I686_ELF_TOOLS}"
echo "BINUTILS_VERSION = ${BINUTILS_VERSION}"
echo "GCC_VERSION      = ${GCC_VERSION}"
echo "PARALLEL         = ${PARALLEL}"
echo "ADD TO BASHRC    = ${ADD_PATH}"
echo "USE AS DEFAULT   = ${USE_DEFAULT}"
echo ""


function main
{
    mkdir -p "$SYSROOT"
    mkdir -p "$SYSROOT/src"
    mkdir -p "$TESTDIR"

    if [[ $I686_ELF_TOOLS == true ]]; then
        if [[ $DOWNLOAD_I686_ELF_TOOLS == true ]]; then
            download_i686_elf
        else
            build_i686_elf
        fi
    fi

    if [[ $NEWLIB == true ]]; then
        echo "Building Newlib"
        build_newlib
    fi

    if [[ $OSTOOLS == true ]]; then
        echo "Building OSTOOLS"
        build_os_binutils
        build_os_gcc
    fi

    if [[ $ADD_PATH == true ]]; then
        echo "Adding the paths of executables to PATH variable"
        install_to_bashrc
    fi

    if [[ $PACKAGE_PATH == 0 ]]; then
        echo ""
    else
        package "$PACKAGE_PATH"
    fi

    echo "Building complete. You may want to change the Tooldir to $SYSROOT/bin in the Makefile in Sectoros-RW4 root directory."
}

main