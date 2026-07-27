#include "geometry.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <utility>
#include <vector>

struct Light {
  Vec3f position;
  float intensity;
  Light(const Vec3f& p, const float& i) : position(p), intensity(i) {}
};

struct Material {
  Vec3f diffuse_color;
  Vec4f albedo;
  float specural_exponant;
  float refractive_index;
  Material(const float& r,
           const Vec4f& a,
           const Vec3f& color,
           const float& spec)
      : refractive_index(r), albedo(a), specural_exponant(spec),
        diffuse_color(color) {}
  Material()
      : refractive_index(1), albedo(1, 0, 0, 0), diffuse_color(),
        specural_exponant() {}
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

Vec3f reflect(const Vec3f& I, const Vec3f& N) { return I - N * 2.f * (I * N); }

Vec3f refract(const Vec3f& I, const Vec3f& N, const float& refractive_index) {
  float cosi = -std::max(-1.f, std::min(1.f, I * N));
  float etai = 1, etat = refractive_index;
  Vec3f n = N;
  if (cosi < 0) {
    cosi = -cosi;
    std::swap(etai, etat);
    n = -N;
  }

  float eta = etai / etat;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  return k < 0 ? Vec3f(0, 0, 0) : I * eta + n * (eta * cosi - sqrtf(k));
}

bool scene_intersect(const Vec3f& orig,
                     const Vec3f& dir,
                     const std::vector<Sphere>& spheres,
                     Vec3f& hit,
                     Vec3f& N,
                     Material& material) {
  float sphere_distance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++) {
    float dist_i;
    if (spheres[i].ray_intersect(orig, dir, dist_i) &&
        dist_i < sphere_distance) {
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
               const std::vector<Light>& lights,
               size_t depth = 0) {
  Vec3f point, N;
  Material material;

  Vec3f bg_color = Vec3f(0.2, 0.7, 0.8);

  if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material)) {
    return bg_color;
  }

  Vec3f reflect_dir = reflect(dir, N).normalize();
  Vec3f refract_dir = refract(dir, N, material.refractive_index).normalize();

  Vec3f reflect_orig =
      reflect_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
  Vec3f refract_orig =
      refract_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;

  Vec3f reflect_color =
      cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
  Vec3f refract_color =
      cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);

  float diffuse_light_intensity = 0, spectural_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++) {
    Vec3f light_dir = (lights[i].position - point).normalize();
    float light_dist = (lights[i].position - point).norm();

    Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
    Vec3f shadow_pt, shadow_N;
    Material m;
    if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N,
                        m) &&
        (shadow_pt - shadow_orig).norm() < light_dist) {
      continue;
    }

    diffuse_light_intensity +=
        lights[i].intensity * std::max(0.f, light_dir * N);
    spectural_light_intensity +=
        powf(std::max(0.f, -reflect(-light_dir, N) * dir),
             material.specural_exponant) *
        lights[i].intensity;
  }

  return material.diffuse_color * diffuse_light_intensity * material.albedo[0] +
         Vec3f(1., 1., 1.) * material.albedo[1] * spectural_light_intensity +
         reflect_color * material.albedo[2] +
         refract_color * material.albedo[3];
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
    Vec3f& c = framebuffer[i];
    float max = std::max(c[0], std::max(c[1], c[2]));
    if (max > 1) c = c * (1. / max);
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }

  ofs.close();
}

int main() {
  Material ivory(1.0, Vec4f(0.9, 0.1, 0.05, 0.0), Vec3f(0.4, 0.4, 0.3), 30);
  Material red(1.0, Vec4f(0.8, 0.5, 0.05, 0.0), Vec3f(0.75, 0.10, 0.20), 80);
  Material blue(1.0, Vec4f(0.7, 0.3, 0.3, 0.0), Vec3f(0.10, 0.20, 0.75), 150);
  Material green(1.0, Vec4f(0.9, 0.2, 0.0, 0.0), Vec3f(0.10, 0.65, 0.20), 40);
  Material yellow(1.0, Vec4f(0.6, 0.9, 0.05, 0.0), Vec3f(0.80, 0.75, 0.15),
                  200);
  Material cyan(1.0, Vec4f(0.8, 0.6, 0.05, 0.0), Vec3f(0.10, 0.70, 0.70), 120);
  Material purple(1.0, Vec4f(0.75, 0.7, 0.05, 0.0), Vec3f(0.55, 0.20, 0.70),
                  100);
  Material orange(1.0, Vec4f(0.85, 0.4, 0.05, 0.0), Vec3f(0.85, 0.45, 0.10),
                  60);
  Material mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425);
  Material glass(1.5, Vec4f(0.0, 0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8), 125.);

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-4.5, 2.0, -18), 2.5, blue));
  spheres.push_back(Sphere(Vec3f(2.2, -2.0, -13), 1.8, red));
  spheres.push_back(Sphere(Vec3f(2.2, -3.0, -10), 1.8, glass));
  spheres.push_back(Sphere(Vec3f(0.0, 1.5, -22), 3.2, green));
  spheres.push_back(Sphere(Vec3f(5.8, -0.8, -17), 2.7, orange));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
  lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
  lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

  render(spheres, lights);

  return 0;
}
