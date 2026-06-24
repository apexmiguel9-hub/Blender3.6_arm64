# Platform configuration for Android cross-compilation

# Zlib - required
find_package(ZLIB REQUIRED)

# JPEG
find_package(JPEG QUIET)

# PNG (requires Zlib)
if(PNG_FOUND)
    # already found
else()
    find_package(PNG QUIET)
endif()

# Freetype (requires PNG and Zlib)
find_package(Freetype QUIET)

# International (ICU)
find_package(ICU QUIET)
