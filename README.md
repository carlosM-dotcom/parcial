# Algoritmos de Rasterización con ImGui

Aplicación interactiva en C++17/OpenGL 3.3 que implementa algoritmos clásicos de rasterización sobre una cuadrícula y permite transformar primitivas mediante Dear ImGui.

## Funcionalidades

- Línea con algoritmo de Bresenham
- Circunferencia con algoritmo del punto medio
- Triángulo relleno mediante barrido horizontal
- Cuadrado relleno
- Forma libre rellena por escaneo
- Cuadrícula configurable
- Traslación, rotación y escala
- Interacción mediante clic sobre el lienzo

## Controles

1. Selecciona una herramienta en el panel izquierdo.
2. Haz clic sobre el lienzo para agregar vértices.
3. Usa «Usar dibujo como selección» para seleccionar la forma.
4. Ajusta traslación, rotación y escala y pulsa «Aplicar transformación».

## Requisitos

- Windows 10/11
- Visual Studio 2022 con «Desarrollo para el escritorio con C++»
- CMake 3.20+
- Ninja
- vcpkg
- Drivers OpenGL 3.3+

## Compilar

```powershell
$env:VCPKG_ROOT="C:\ruta\a\vcpkg"
cmake --preset windows-vcpkg
cmake --build build --config Release
.\build\Release\raster_app.exe
```

Si CMake no encuentra `vcpkg.cmake`, instala las dependencias manualmente:

```powershell
vcpkg install glfw3 glad imgui[glfw-binding,opengl3-binding]:x64-windows
```

## Dependencias

- GLFW
- GLAD
- Dear ImGui
- OpenGL

## Licencia

MIT. Consulta [LICENSE](LICENSE).
