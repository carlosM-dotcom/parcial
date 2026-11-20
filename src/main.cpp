#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>
#include <vector>

constexpr int CANVAS_W = 640;
constexpr int CANVAS_H = 480;
constexpr int CELL = 20;
constexpr int COLS = CANVAS_W / CELL;
constexpr int ROWS = CANVAS_H / CELL;

struct Point { int x; int y; };

struct PixelCanvas {
    std::vector<unsigned char> pixels;
    unsigned int texture = 0;

    PixelCanvas() : pixels(CANVAS_W * CANVAS_H * 4) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        clear();
    }

    void clear() {
        std::fill(pixels.begin(), pixels.end(), 0);
    }

    void set(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
        if (x < 0 || x >= CANVAS_W || y < 0 || y >= CANVAS_H) return;
        int i = (y * CANVAS_W + x) * 4;
        pixels[i] = r; pixels[i + 1] = g; pixels[i + 2] = b; pixels[i + 3] = a;
    }

    void upload() {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CANVAS_W, CANVAS_H, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }

    ~PixelCanvas() { if (texture) glDeleteTextures(1, &texture); }
};

static void drawGrid(PixelCanvas& canvas) {
    canvas.clear();
    for (int y = 0; y < CANVAS_H; ++y)
        for (int x = 0; x < CANVAS_W; ++x)
            canvas.set(x, y, 18, 22, 28);
    for (int x = 0; x <= CANVAS_W; x += CELL)
        for (int y = 0; y < CANVAS_H; ++y)
            canvas.set(x, y, 55, 62, 72);
    for (int y = 0; y <= CANVAS_H; y += CELL)
        for (int x = 0; x < CANVAS_W; ++x)
            canvas.set(x, y, 55, 62, 72);
}

static void bresenham(PixelCanvas& canvas, Point a, Point b,
                      unsigned char r = 255, unsigned char g = 220, unsigned char blue = 80) {
    int dx = std::abs(b.x - a.x), sx = a.x < b.x ? 1 : -1;
    int dy = -std::abs(b.y - a.y), sy = a.y < b.y ? 1 : -1;
    int error = dx + dy;
    while (true) {
        canvas.set(a.x, a.y, r, g, blue);
        if (a.x == b.x && a.y == b.y) break;
        int e2 = 2 * error;
        if (e2 >= dy) { error += dy; a.x += sx; }
        if (e2 <= dx) { error += dx; a.y += sy; }
    }
}

static void circleMidpoint(PixelCanvas& canvas, Point center, int radius,
                           unsigned char r = 100, unsigned char g = 220, unsigned char b = 255) {
    int x = 0, y = radius, p = 1 - radius;
    auto plot = [&](int px, int py) {
        canvas.set(center.x + px, center.y + py, r, g, b);
        canvas.set(center.x - px, center.y + py, r, g, b);
        canvas.set(center.x + px, center.y - py, r, g, b);
        canvas.set(center.x - px, center.y - py, r, g, b);
        canvas.set(center.x + py, center.y + px, r, g, b);
        canvas.set(center.x - py, center.y + px, r, g, b);
        canvas.set(center.x + py, center.y - px, r, g, b);
        canvas.set(center.x - py, center.y - px, r, g, b);
    };
    while (x <= y) {
        plot(x, y);
        ++x;
        if (p < 0) p += 2 * x + 1;
        else { --y; p += 2 * (x - y) + 1; }
    }
}

static void fillTriangle(PixelCanvas& canvas, Point a, Point b, Point c) {
    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);
    auto fillSpan = [&](int y, int x1, int x2) {
        if (x1 > x2) std::swap(x1, x2);
        for (int x = x1; x <= x2; ++x) canvas.set(x, y, 255, 140, 80);
    };
    for (int y = a.y; y <= c.y; ++y) {
        float t1 = c.y == a.y ? 0.0f : (float)(y - a.y) / (c.y - a.y);
        float t2 = b.y >= a.y && y <= b.y ? (float)(y - a.y) / (b.y - a.y)
                                          : (float)(y - b.y) / (c.y - b.y);
        int x1 = (int)std::round(a.x + t1 * (c.x - a.x));
        int x2 = y <= b.y ? (int)std::round(a.x + t2 * (b.x - a.x))
                          : (int)std::round(b.x + t2 * (c.x - b.x));
        fillSpan(y, x1, x2);
    }
    bresenham(canvas, a, b); bresenham(canvas, b, c); bresenham(canvas, c, a);
}

static void fillSquare(PixelCanvas& canvas, Point a, Point b) {
    int x1 = std::min(a.x, b.x), x2 = std::max(a.x, b.x);
    int y1 = std::min(a.y, b.y), y2 = std::max(a.y, b.y);
    for (int y = y1; y <= y2; ++y)
        for (int x = x1; x <= x2; ++x)
            canvas.set(x, y, 255, 190, 70);
    bresenham(canvas, {x1, y1}, {x2, y1}, 255, 240, 180);
    bresenham(canvas, {x2, y1}, {x2, y2}, 255, 240, 180);
    bresenham(canvas, {x2, y2}, {x1, y2}, 255, 240, 180);
    bresenham(canvas, {x1, y2}, {x1, y1}, 255, 240, 180);
}

static void fillPolygon(PixelCanvas& canvas, const std::vector<Point>& poly) {
    if (poly.size() < 3) return;
    int yMin = poly[0].y, yMax = poly[0].y;
    for (const auto& p : poly) { yMin = std::min(yMin, p.y); yMax = std::max(yMax, p.y); }
    for (int y = yMin; y <= yMax; ++y) {
        std::vector<int> xs;
        for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
            int x1 = poly[i].x, y1 = poly[i].y;
            int x2 = poly[j].x, y2 = poly[j].y;
            if ((y1 <= y && y < y2) || (y2 <= y && y < y1)) {
                xs.push_back(x1 + (y - y1) * (x2 - x1) / (y2 - y1));
            }
        }
        std::sort(xs.begin(), xs.end());
        for (size_t i = 0; i + 1 < xs.size(); i += 2)
            for (int x = xs[i]; x <= xs[i + 1]; ++x)
                canvas.set(x, y, 160, 120, 255);
    }
    for (size_t i = 0; i < poly.size(); ++i)
        bresenham(canvas, poly[i], poly[(i + 1) % poly.size()], 220, 220, 255);
}

static void transformPoints(std::vector<Point>& points, float dx, float dy, float angleDeg, float scale) {
    if (points.empty()) return;
    float cx = 0, cy = 0;
    for (const auto& p : points) { cx += p.x; cy += p.y; }
    cx /= (float)points.size(); cy /= (float)points.size();
    float a = angleDeg * 3.14159265f / 180.0f;
    for (auto& p : points) {
        float vx = p.x - cx, vy = p.y - cy;
        float rx = vx * std::cos(a) - vy * std::sin(a);
        float ry = vx * std::sin(a) + vy * std::cos(a);
        p.x = (int)std::round(cx + rx * scale + dx);
        p.y = (int)std::round(cy + ry * scale + dy);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1100, 650, "Algoritmos de Rasterizacion", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    PixelCanvas canvas;
    enum class Mode { Line, Circle, Triangle, Square, Free } mode = Mode::Line;
    std::vector<Point> pending, shape;
    float translate[2] = {0, 0};
    float rotation = 0, scale = 1.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawGrid(canvas);
        if (mode == Mode::Line && pending.size() == 2)
            bresenham(canvas, pending[0], pending[1]);
        if (mode == Mode::Circle && pending.size() == 2) {
            int r = (int)std::round(std::hypot(pending[1].x - pending[0].x, pending[1].y - pending[0].y));
            circleMidpoint(canvas, pending[0], r);
        }
        if (mode == Mode::Triangle && pending.size() == 3)
            fillTriangle(canvas, pending[0], pending[1], pending[2]);
        if (mode == Mode::Square && pending.size() == 2)
            fillSquare(canvas, pending[0], pending[1]);
        if (mode == Mode::Free && pending.size() >= 3)
            fillPolygon(canvas, pending);

        if (!shape.empty()) {
            for (size_t i = 0; i < shape.size(); ++i)
                bresenham(canvas, shape[i], shape[(i + 1) % shape.size()], 255, 80, 120);
        }
        canvas.upload();

        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("Herramientas", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Algoritmos y primitivas");
        if (ImGui::Button("Linea Bresenham")) { mode = Mode::Line; pending.clear(); }
        ImGui::SameLine();
        if (ImGui::Button("Circulo")) { mode = Mode::Circle; pending.clear(); }
        if (ImGui::Button("Triangulo relleno")) { mode = Mode::Triangle; pending.clear(); }
        ImGui::SameLine();
        if (ImGui::Button("Cuadrado")) { mode = Mode::Square; pending.clear(); }
        if (ImGui::Button("Forma libre")) { mode = Mode::Free; pending.clear(); }
        ImGui::SameLine();
        if (ImGui::Button("Limpiar")) { pending.clear(); shape.clear(); }

        ImGui::Separator();
        ImGui::Text("Transformaciones");
        ImGui::SliderFloat("Trasladar X", &translate[0], -200.0f, 200.0f);
        ImGui::SliderFloat("Trasladar Y", &translate[1], -200.0f, 200.0f);
        ImGui::SliderFloat("Rotacion", &rotation, -180.0f, 180.0f);
        ImGui::SliderFloat("Escala", &scale, 0.25f, 3.0f);
        if (ImGui::Button("Aplicar transformacion") && !shape.empty())
            transformPoints(shape, translate[0], translate[1], rotation, scale);
        if (ImGui::Button("Usar dibujo como seleccion") && !pending.empty()) {
            shape = pending;
        }

        ImGui::Separator();
        ImGui::Text("Pixeles: %d x %d  |  Celdas: %d x %d", CANVAS_W, CANVAS_H, COLS, ROWS);
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(280, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("Lienzo", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((ImTextureID)(intptr_t)canvas.texture, ImVec2(CANVAS_W, CANVAS_H));
        if (ImGui::IsItemClicked()) {
            ImVec2 mouse = ImGui::GetMousePos();
            ImVec2 min = ImGui::GetItemRectMin();
            int gx = (int)((mouse.x - min.x) / (CANVAS_W / (float)COLS));
            int gy = (int)((mouse.y - min.y) / (CANVAS_H / (float)ROWS));
            Point p{std::clamp(gx * CELL + CELL / 2, 0, CANVAS_W - 1),
                    std::clamp(gy * CELL + CELL / 2, 0, CANVAS_H - 1)};
            int limit = mode == Mode::Triangle ? 3 : (mode == Mode::Free ? 100 : 2);
            if (mode == Mode::Free) pending.push_back(p);
            else if ((int)pending.size() < limit) pending.push_back(p);
            else pending = {p};
        }
        ImGui::End();

        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
