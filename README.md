# Cobalt

**One unified Android renderer for Minecraft: Java Edition**

Cobalt presents a single renderer identity to users and launchers while internally selecting the appropriate implementation family based on Minecraft version and device capabilities.

## Routing Policy

| Minecraft Version | Route Family | Backend | Objective |
|---|---|---|---|
| ≤ 1.17.0 | **Legacy** | gl4es_extra_extra (GLES 1.1/2.0) | Older OpenGL, fixed-function compatibility, stability |
| > 1.17.0 | **Modern** | MobileGlues (GLES 3.x) | Raw performance, modern shaders, efficient chunk rendering |

## Project Structure

```
Cobalt/
├── cobalt-core/       # Public API, data models, routing, all implementations
├── cobalt-launcher/   # Launcher adapter contract and plugin integration
├── cobalt-legacy/     # Legacy route adapter with gl4es JNI bridge
├── cobalt-modern/     # Modern route adapter with MobileGlues JNI bridge
└── cobalt-app/        # Standalone APK application module
```

## Building

```bash
# Requires: Android SDK 35, NDK 27.x, JDK 17
./gradlew assembleDebug        # Debug APK (all ABIs)
./gradlew assembleRelease      # Release APK (all ABIs)
```

## CI/CD

GitHub Actions builds on every push to `main` and `develop`. Artifacts include debug and release APKs for arm64-v8a and universal builds.

## Requirements

- Android SDK 35
- NDK 27.x
- Kotlin 2.1.0
- JDK 17
- Min SDK 24 (Android 7.0)

## References

| Repository | Purpose |
|---|---|
| [gl4es_extra_extra](https://github.com/artdeell/gl4es_extra_extra/) | Legacy compatibility path |
| [NGG-FCLRendererPlugin](https://github.com/BZLZHH/NGG-FCLRendererPlugin) | Plugin packaging reference |
| [MobileGlues-release](https://github.com/MobileGL-Dev/MobileGlues-release) | Modern compatibility path |

## License

MIT — see [LICENSE](LICENSE). Third-party components retain their original licenses — see [NOTICE](NOTICE).
