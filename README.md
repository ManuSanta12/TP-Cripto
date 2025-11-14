# TP-Cripto

## Build

Script recomendado para compilar en un checkout limpio:

```bash
./scripts/build.sh            # Configura y compila (Release)
./scripts/build.sh clean      # Limpia objetos compilados en build/
./scripts/build.sh distclean  # Elimina por completo build/
```

Notas:

- En macOS con Homebrew, el script intenta ubicar OpenSSL automáticamente.
- El binario resultante queda en `build/stegobmp` (con generadores estándar).

Compilación manual (alternativa):

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
