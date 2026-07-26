#include "geometry.h"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <vector>

using std::vector;

void render(void) {
  const int WIDTH = 1024;
  const int HEIGHT = 768;
  vector<Vec3f> framebuffer(WIDTH * HEIGHT);

  for (size_t h = 0; h < HEIGHT; h++) {
    for (size_t w = 0; w < WIDTH; w++) {
      framebuffer[w + h * WIDTH] =
          Vec3f(h / float(HEIGHT), w / float(WIDTH), 0);
    }
  }

  std::ofstream ofs;
  ofs.open("./out.ppm");
  ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
  for (size_t i = 0; i < WIDTH * HEIGHT; ++i) {
    for (size_t c = 0; c < 3; c++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][c])));
    }
  }

  ofs.close();
}

int main() {
  render();
  return 0;
}
