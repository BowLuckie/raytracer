#include "geometry.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <vector>

struct Light {
  Vec3f position;
  float intensity;
  Light(const Vec3f& p, const float& i) : position(p), intensity(i) {}
};

struct Material {
  Vec3f diffuse_color;
  Material(const Vec3f& color) : diffuse_color(color) {}
  Material() : diffuse_color() {}
};

struct Sphere {
  Vec3f center;
  float radius;
  Material material;

  Sphere(const Vec3f& c, const float& r, const Material& m)
      : center(c), radius(r), material(m) {}

  bool ray_intersect(const Vec3f& orig, const Vec3f& dir, float& t0) const {
    Vec3f L = center - orig;
    float tca = L * dir;
    float d2 = L * L - tca * tca;
    if (d2 > radius * radius) return false;
    float thc = std::sqrtf(radius * radius - d2);
    t0 = tca - thc;
    float t1 = tca + thc;
    if (t0 < 0) t0 = t1;
    if (t0 < 0) return false;
    return true;
  }
};

bool scene_intersect(const Vec3f& orig,
                     const Vec3f& dir,
                     const std::vector<Sphere>& spheres,
                     Vec3f& hit,
                     Vec3f& N,
                     Material& material) {
  float sphere_distance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++) {
    float dist_i;
    if (spheres[i].ray_intersect(orig, dir, dist_i)) {
      sphere_distance = dist_i;
      hit = orig + dir * dist_i;
      N = (hit - spheres[i].center).normalize();
      material = spheres[i].material;
    }
  }
  return sphere_distance < 1000;
}

Vec3f cast_ray(const Vec3f& orig,
               const Vec3f& dir,
               const std::vector<Sphere>& spheres,
               const std::vector<Light>& lights) {
  Vec3f point, N;
  Material material;

  Vec3f bg_color = Vec3f(0.2, 0.7, 0.8);

  if (!scene_intersect(orig, dir, spheres, point, N, material)) {
    return bg_color;
  }

  float diffuse_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++) {
    Vec3f light_dir = (lights[i].position - point).normalize();
    diffuse_light_intensity +=
        lights[i].intensity * std::max(0.f, light_dir * N);
  }

  return material.diffuse_color * diffuse_light_intensity;
}

void render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights) {
  const int WIDTH = 1024;
  const int HEIGHT = 768;
  const int fov = M_PI / 2;
  std::vector<Vec3f> framebuffer(WIDTH * HEIGHT);

  for (size_t h = 0; h < HEIGHT; h++) {
    for (size_t w = 0; w < WIDTH; w++) {
      float x = (2 * (w + 0.5) / (float)WIDTH - 1) * tan(fov / 2.) * WIDTH /
                (float)HEIGHT;
      float y = -(2 * (h + 0.5) / (float)HEIGHT - 1) * tan(fov / 2.);
      Vec3f dir = Vec3f(x, y, -1).normalize();
      framebuffer[w + h * WIDTH] =
          cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
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
  Material ivory(Vec3f(0.4, 0.4, 0.3));
  Material red(Vec3f(0.75, 0.10, 0.20));
  Material blue(Vec3f(0.10, 0.20, 0.75));
  Material green(Vec3f(0.10, 0.65, 0.20));
  Material yellow(Vec3f(0.80, 0.75, 0.15));
  Material cyan(Vec3f(0.10, 0.70, 0.70));
  Material purple(Vec3f(0.55, 0.20, 0.70));
  Material orange(Vec3f(0.85, 0.45, 0.10));
  Material pink(Vec3f(0.85, 0.40, 0.65));
  Material gray(Vec3f(0.45, 0.45, 0.45));
  Material dark_blue(Vec3f(0.10, 0.10, 0.25));
  Material dark_green(Vec3f(0.10, 0.25, 0.10));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-4.5, 2.0, -18), 2.5, blue));
  spheres.push_back(Sphere(Vec3f(2.2, -2.0, -13), 1.8, red));
  spheres.push_back(Sphere(Vec3f(0.0, 1.5, -22), 3.2, green));
  spheres.push_back(Sphere(Vec3f(5.8, -0.8, -17), 2.7, orange));

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));

  render(spheres, lights);

  return 0;
}
