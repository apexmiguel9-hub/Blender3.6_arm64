#!/usr/bin/env python3
"""
Apply Android modifications to Blender 2.79b source tree.
Usage: python3 apply_patches.py <blender_dir> <patch_dir>
"""
import os
import sys
import shutil

def patch_file(filepath, old, new):
    with open(filepath, 'r') as f:
        content = f.read()
    if old not in content:
        print(f"  WARNING: Pattern not found in {filepath}")
        print(f"    Looking for: {old[:80]}...")
        return False
    content = content.replace(old, new, 1)
    with open(filepath, 'w') as f:
        f.write(content)
    print(f"  Patched: {filepath}")
    return True

def main():
    blender_dir = sys.argv[1]
    patch_dir = sys.argv[2]  # directory with GHOST files

    ghost_src = os.path.join(patch_dir, 'android', 'ghost')

    # 1. Copy GHOST Android backend files
    print("[1] Copying GHOST Android backend files...")
    for f in ['GHOST_SystemAndroid.h', 'GHOST_SystemAndroid.cpp',
              'GHOST_WindowAndroid.h', 'GHOST_WindowAndroid.cpp']:
        src = os.path.join(ghost_src, f)
        dst = os.path.join(blender_dir, 'intern', 'ghost', 'intern', f)
        shutil.copy2(src, dst)
        print(f"    Copied {f}")

    # 1.5. Copy platform_android.cmake
    print("[1.5] Copying platform_android.cmake...")
    src = os.path.join(patch_dir, 'config', 'platform_android.cmake')
    dst = os.path.join(blender_dir, 'build_files', 'cmake', 'platform', 'platform_android.cmake')
    if os.path.exists(src):
        shutil.copy2(src, dst)
        print(f"    Copied platform_android.cmake")
    else:
        print(f"    WARNING: platform_android.cmake not found at {src}")

    # 2. Modify GHOST_ISystem.cpp - add Android system creation
    print("[2] Patching GHOST_ISystem.cpp...")
    ghost_isystem = os.path.join(blender_dir, 'intern', 'ghost', 'intern', 'GHOST_ISystem.cpp')
    with open(ghost_isystem, 'r') as f:
        content = f.read()

    # Add include
    old_inc = '#include "GHOST_SystemSDL.h"'
    new_inc = '#include "GHOST_SystemSDL.h"\n#include "GHOST_SystemAndroid.h"'
    content = content.replace(old_inc, new_inc)

    # Add Android case in createSystem()
    old_sys = '#ifdef WITH_X11\n\t\tm_system = new GHOST_SystemX11();'
    new_sys = '#ifdef WITH_X11\n\t\tm_system = new GHOST_SystemX11();\n#elif defined(WITH_GHOST_ANDROID)\n\t\tm_system = new GHOST_SystemAndroid();'
    content = content.replace(old_sys, new_sys)

    with open(ghost_isystem, 'w') as f:
        f.write(content)
    print("    Patched GHOST_ISystem.cpp")

    # 3. Modify intern/ghost/CMakeLists.txt
    print("[3] Patching intern/ghost/CMakeLists.txt...")
    ghost_cmake = os.path.join(blender_dir, 'intern', 'ghost', 'CMakeLists.txt')
    with open(ghost_cmake, 'r') as f:
        content = f.read()

    # Add WITH_GHOST_ANDROID define
    old = 'if(WITH_HEADLESS)\n\tadd_definitions(-DWITH_GHOST_NULL)\nendif()'
    new = '''if(WITH_HEADLESS)
	add_definitions(-DWITH_GHOST_NULL)
endif()

if(WITH_ANDROID_MODULE)
	add_definitions(-DWITH_GHOST_ANDROID)
	add_definitions(-DWITH_GL_EGL)
endif()'''
    content = content.replace(old, new)

    # Add Android source files after WIN32 section
    old = 'if(WIN32)'
    new = '''if(WITH_ANDROID_MODULE)
	list(APPEND SRC
		intern/GHOST_SystemAndroid.cpp
		intern/GHOST_SystemAndroid.h
		intern/GHOST_WindowAndroid.cpp
		intern/GHOST_WindowAndroid.h
	)
endif()

if(WIN32)'''
    content = content.replace(old, new)

    with open(ghost_cmake, 'w') as f:
        f.write(content)
    print("    Patched intern/ghost/CMakeLists.txt")

    # 4. Modify root CMakeLists.txt
    print("[4] Patching root CMakeLists.txt...")
    root_cmake = os.path.join(blender_dir, 'CMakeLists.txt')
    with open(root_cmake, 'r') as f:
        content = f.read()

    # Add WITH_ANDROID_MODULE option after WITH_PYTHON_MODULE
    old = 'option(WITH_PYTHON_MODULE "Enable building as a python module'
    new = '''option(WITH_ANDROID_MODULE "Build Blender as Android shared library (.so)" OFF)
mark_as_advanced(WITH_ANDROID_MODULE)

option(WITH_PYTHON_MODULE "Enable building as a python module'''
    content = content.replace(old, new)

    # When WITH_ANDROID_MODULE, don't force headless
    old = '''if(WITH_PYTHON_MODULE)
	set(WITH_HEADLESS ON)
	set(WITH_PLAYER OFF)
	set(WITH_CYCLES OFF)
	set(WITH_COMPOSITOR OFF)
endif()

if(WITH_PYTHON_MODULE)'''
    new = '''if(WITH_ANDROID_MODULE)
	set(WITH_HEADLESS OFF)
	set(WITH_PLAYER OFF)
elseif(WITH_PYTHON_MODULE)
	set(WITH_HEADLESS ON)
	set(WITH_PLAYER OFF)
	set(WITH_CYCLES OFF)
	set(WITH_COMPOSITOR OFF)
endif()

if(WITH_PYTHON_MODULE OR WITH_ANDROID_MODULE)'''
    content = content.replace(old, new)

    # Add GL4ES support in the OpenGL section
    old = '# Configure OpenGL.\n\nfind_package(OpenGL)'
    new = '''# Configure OpenGL.

if(WITH_ANDROID_MODULE)
	# Use GL4ES for Android (provides desktop GL on top of GLES)
	set(GL4ES_LIBRARY "" CACHE FILEPATH "GL4ES libGL.a (static)")
	set(GL4ES_INCLUDE_DIR "" CACHE PATH "GL4ES headers")
	mark_as_advanced(GL4ES_LIBRARY GL4ES_INCLUDE_DIR)

	set(OPENGL_FOUND TRUE)
	set(OPENGL_gl_LIBRARY "${GL4ES_LIBRARY}")
	set(OPENGL_INCLUDE_DIR "${GL4ES_INCLUDE_DIR}")
	set(OPENGL_glu_LIBRARY "")
else()
	find_package(OpenGL)
endif()'''
    content = content.replace(old, new)

    # Disable X11 for Android
    old = 'if(WITH_GHOST_SDL OR WITH_HEADLESS)\n\tset(WITH_X11           OFF)\n\tset(WITH_X11_XINPUT    OFF)\n\tset(WITH_X11_XF86VMODE OFF)\n\tset(WITH_X11_XFIXES    OFF)\n\tset(WITH_X11_ALPHA     OFF)\n\tset(WITH_GHOST_XDND    OFF)\n\tset(WITH_INPUT_IME     OFF)\nendif()'
    new = '''if(WITH_GHOST_SDL OR WITH_HEADLESS OR WITH_ANDROID_MODULE)
	set(WITH_X11           OFF)
	set(WITH_X11_XINPUT    OFF)
	set(WITH_X11_XF86VMODE OFF)
	set(WITH_X11_XFIXES    OFF)
	set(WITH_X11_ALPHA     OFF)
	set(WITH_GHOST_XDND    OFF)
	set(WITH_INPUT_IME     OFF)
endif()'''
    content = content.replace(old, new)

    # Skip platform_unix.cmake for Android (handles deps like JPEG/PNG/Freetype)
    old = 'if(UNIX AND NOT APPLE)\n\tinclude(platform_unix)'
    new = '''if(UNIX AND NOT APPLE)
	if(WITH_ANDROID_MODULE)
		include(platform_android)
	else()
		include(platform_unix)
	endif()'''
    content = content.replace(old, new)

    # Add EGL/GLESv2 linkage for Android (needed by GL4ES)
    old = 'if(WITH_GL_PROFILE_COMPAT OR WITH_GL_PROFILE_CORE)\n\tlist(APPEND BLENDER_GL_LIBRARIES "${OPENGL_gl_LIBRARY}")'
    new = '''if(WITH_ANDROID_MODULE)
	# GL4ES needs system EGL and GLESv2 at runtime
	list(APPEND BLENDER_GL_LIBRARIES "-lEGL" "-lGLESv2")
	list(APPEND GL_DEFINITIONS -DWITH_GL_EGL -DWITH_EGL -DWITH_GL_PROFILE_COMPAT)
endif()

if(WITH_GL_PROFILE_COMPAT OR WITH_GL_PROFILE_CORE)
	list(APPEND BLENDER_GL_LIBRARIES "${OPENGL_gl_LIBRARY}")'''
    content = content.replace(old, new)

    with open(root_cmake, 'w') as f:
        f.write(content)
    print("    Patched root CMakeLists.txt")

    # 5. Modify source/creator/CMakeLists.txt
    print("[5] Patching source/creator/CMakeLists.txt...")
    creator_cmake = os.path.join(blender_dir, 'source', 'creator', 'CMakeLists.txt')
    with open(creator_cmake, 'r') as f:
        content = f.read()

    # Add ANDROID_MODULE define
    old = 'if(WITH_PYTHON_MODULE)\n\tadd_definitions(-DWITH_PYTHON_MODULE)'
    new = '''if(WITH_ANDROID_MODULE)
	add_definitions(-DWITH_ANDROID_MODULE)
	add_definitions(-DWITH_PYTHON_MODULE)
endif()

if(WITH_PYTHON_MODULE)
	add_definitions(-DWITH_PYTHON_MODULE)'''
    content = content.replace(old, new)

    # Build as shared library for Android
    old = '''else()
	add_executable(blender ${EXETYPE} ${SRC})
	WINDOWS_SIGN_TARGET(blender)
endif()'''
    new = '''elseif(WITH_ANDROID_MODULE)
	add_library(blender SHARED ${SRC})
	set_target_properties(
		blender
		PROPERTIES
			PREFIX "lib"
			OUTPUT_NAME blender
			LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	)
else()
	add_executable(blender ${EXETYPE} ${SRC})
	WINDOWS_SIGN_TARGET(blender)
endif()'''
    content = content.replace(old, new)

    with open(creator_cmake, 'w') as f:
        f.write(content)
    print("    Patched source/creator/CMakeLists.txt")

    # 6. Modify creator.c
    print("[6] Patching source/creator/creator.c...")
    creator_c = os.path.join(blender_dir, 'source', 'creator', 'creator.c')
    with open(creator_c, 'r') as f:
        content = f.read()

    content = content.replace(
        '#ifdef WITH_PYTHON_MODULE\n/* allow python module to call main */\n#  define main main_python_enter',
        '#if defined(WITH_PYTHON_MODULE) || defined(WITH_ANDROID_MODULE)\n/* allow calling main as library function */\n#  define main main_python_enter'
    )

    # Fix WIN32 arg handling for Android
    content = content.replace(
        'int main(\n        int argc,\n#ifdef WIN32\n        const char **UNUSED(argv_c)\n#else\n        const char **argv\n#endif\n        )',
        'int main(\n        int argc,\n#if defined(WIN32) && !defined(WITH_ANDROID_MODULE)\n        const char **UNUSED(argv_c)\n#else\n        const char **argv\n#endif\n        )'
    )

    with open(creator_c, 'w') as f:
        f.write(content)
    print("    Patched source/creator/creator.c")

    # 7. Fix malloc_stats() not available on Android (bionic libc)
    print("[7] Patching guardedalloc for Android...")
    for f in ['mallocn_lockfree_impl.c', 'mallocn_guarded_impl.c']:
        alloc_file = os.path.join(blender_dir, 'intern', 'guardedalloc', 'intern', f)
        with open(alloc_file, 'r') as fh:
            content = fh.read()
        content = content.replace(
            '#include "mallocn_intern.h"',
            '#include "mallocn_intern.h"\n#ifdef __ANDROID__\n/* bionic libc does not have malloc_stats */\n#define malloc_stats()\n#endif'
        )
        with open(alloc_file, 'w') as fh:
            fh.write(content)
        print(f"    Patched {f}")

    # 8. Static-link host tools (datatoc, makesdna, makesrna) so QEMU can run them
    #    without needing Android system linker (/system/bin/linker64)
    print("[8] Static-linking host tools for QEMU compatibility...")

    # Patch datatoc
    datatoc_cmake = os.path.join(blender_dir, 'source', 'blender', 'datatoc', 'CMakeLists.txt')
    with open(datatoc_cmake, 'r') as fh:
        content = fh.read()

    # datatoc (always built)
    content = content.replace(
        'add_executable(datatoc ${SRC})',
        'add_executable(datatoc ${SRC})\n\tset_target_properties(datatoc PROPERTIES LINK_FLAGS "-static")'
    )

    # datatoc_icon (built when WITH_HEADLESS=OFF)
    content = content.replace(
        'add_executable(datatoc_icon ${SRC})',
        'add_executable(datatoc_icon ${SRC})\n\tset_target_properties(datatoc_icon PROPERTIES LINK_FLAGS "-static")'
    )

    with open(datatoc_cmake, 'w') as fh:
        fh.write(content)
    print("    Patched datatoc/CMakeLists.txt")

    # Patch makesdna
    makesdna_cmake = os.path.join(blender_dir, 'source', 'blender', 'makesdna', 'intern', 'CMakeLists.txt')
    with open(makesdna_cmake, 'r') as fh:
        content = fh.read()
    content = content.replace(
        'add_executable(makesdna ${SRC} ${SRC_DNA_INC})',
        'add_executable(makesdna ${SRC} ${SRC_DNA_INC})\n\tset_target_properties(makesdna PROPERTIES LINK_FLAGS "-static")'
    )
    with open(makesdna_cmake, 'w') as fh:
        fh.write(content)
    print("    Patched makesdna/intern/CMakeLists.txt")

    # Patch makesrna
    makesrna_cmake = os.path.join(blender_dir, 'source', 'blender', 'makesrna', 'intern', 'CMakeLists.txt')
    with open(makesrna_cmake, 'r') as fh:
        content = fh.read()
    content = content.replace(
        'add_executable(makesrna ${SRC} ${SRC_RNA_INC} ${SRC_DNA_INC})',
        'add_executable(makesrna ${SRC} ${SRC_RNA_INC} ${SRC_DNA_INC})\n\tset_target_properties(makesrna PROPERTIES LINK_FLAGS "-static")'
    )
    with open(makesrna_cmake, 'w') as fh:
        fh.write(content)
    print("    Patched makesrna/intern/CMakeLists.txt")

    # 9. Fix Eigen3 Functors.h for NDK r27 (std::binder2nd/1st removed from libc++)
    print("[9] Patching Eigen3 Functors.h for NDK r27 compatibility...")
    eigen_functors = os.path.join(blender_dir, 'extern', 'Eigen3', 'Eigen', 'src', 'Core', 'Functors.h')
    with open(eigen_functors, 'r') as fh:
        content = fh.read()
    content = content.replace(
        'template<typename T>\nstruct functor_traits<std::binder2nd<T> >\n{ enum { Cost = functor_traits<T>::Cost, PacketAccess = false }; };',
        ''
    )
    content = content.replace(
        'template<typename T>\nstruct functor_traits<std::binder1st<T> >\n{ enum { Cost = functor_traits<T>::Cost, PacketAccess = false }; };',
        ''
    )
    with open(eigen_functors, 'w') as fh:
        fh.write(content)
    print("    Patched extern/Eigen3/Eigen/src/Core/Functors.h")

    print("\n=== All Android patches applied successfully ===")


if __name__ == '__main__':
    main()
