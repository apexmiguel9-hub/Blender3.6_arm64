# Blender 2.79b para Android

Build nativo de Blender 2.79b como `.so` para Android ARM64, usando **GL4ES** para traducción OpenGL→GLES y **EGL** como backend de ventana.

## Arquitectura

```
┌─────────────────────────────────────────┐
│           App Android (Java)            │
│  BlenderActivity + SurfaceView         │
├─────────────────────────────────────────┤
│         JNI Wrapper (blender_jni)       │
├─────────────────────────────────────────┤
│       libblender.so (Blender 2.79b)     │
│  ┌───────────────────────────────────┐  │
│  │  GHOST (EGL + Android backend)   │  │
│  │  GPU (OpenGL 2.1 calls)          │  │
│  │  Cycles (CPU path tracer)        │  │
│  │  WM / Editors / BKE / RNA        │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│     libGL4ES.so (OpenGL → GLES)        │
├─────────────────────────────────────────┤
│   Android GLES 2.0/3.0 GPU Drivers     │
└─────────────────────────────────────────┘
```

## Por qué Blender 2.79b?

| Feature | 2.79b | 3.6+ |
|---------|-------|------|
| OpenGL 2.1 (fácil de wrappear con GL4ES) | ✅ | ❌ (3.3+) |
| Cycles ya funcional | ✅ | ✅ |
| UI simple (GHOST) de modificar | ✅ | ❌ |
| Tamaño de código | ~150MB | ~500MB+ |
| OpenGL ES sin geometry shaders | ✅ Parcheable | ❌ |

## Build System

### Con GitHub Actions

Hacer fork, pushear, y el CI buildée automáticamente.

### Local (Linux x86_64 cross-compile)

```bash
# Requisitos: Android NDK r26b+
export ANDROID_NDK_HOME=/path/to/ndk
./scripts/build_android.sh
```

### Modular por contenedores

Elegir perfil:
```bash
./scripts/build_android.sh --config config/modelling.conf   # Solo modelado
./scripts/build_android.sh --config config/animation.conf   # Animación + Cycles
./scripts/build_android.sh --config config/full.conf        # Completo
```

## GL4ES

Se usa GL4ES (no el parche ES2 directo) porque:
- Blender 2.79 usa OpenGL 2.1 → GL4ES traduce a GLES 2.0
- Sin modificar shaders ni código de rendering
- Mejor compatibilidad con GPUs Android
- Se linkea estáticamente dentro del `.so` final

## Estructura del proyecto

```
blender-android/
├── .github/workflows/build.yml   # GitHub Actions CI
├── patches/                       # Parches para Blender 2.79b
│   ├── 0001-build-as-shared-library.patch
│   ├── 0002-ghost-android-egl.patch
│   └── 0003-root-cmake-android-option.patch
├── android/                       # Wrapper Android
│   ├── AndroidManifest.xml
│   ├── CMakeLists.txt
│   ├── jni/blender_jni.cpp       # JNI bridge
│   └── src/.../BlenderActivity.java
├── scripts/
│   └── build_android.sh          # Build script
├── config/                        # Perfiles modulares
│   ├── modelling.conf
│   ├── animation.conf
│   └── full.conf
└── cmake/
    └── android_toolchain.cmake
```

## Roadmap

1. **Fase 1** - Build funcional (GH Actions → .so ARM64 con GL4ES)
2. **Fase 2** - Que abra ventana y renderice (EGL + SurfaceView)
3. **Fase 3** - Input táctil (touch→mouse, gestures)
4. **Fase 4** - UI custom (opcional: Godot como overlay)
5. **Fase 5** - Contenedores modulares (modelado vs animación)
6. **Fase 6** - Optimizaciones Cycles para ARM (NEON, etc.)
