#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>

#include <geometry.hh> // https://github.com/salvipeter/libgeom/
#include <powell.hh>   // https://github.com/salvipeter/dfo/

using namespace Geometry;

DoubleVector affine(const DoubleVector &a, double x, const DoubleVector &b) {
  size_t n = a.size();
  DoubleVector result(n);
  for (size_t i = 0; i < n; ++i)
    result[i] = a[i] * (1 - x) + b[i] * x;
  return result;
}

std::vector<DoubleVector> vertices(size_t n, size_t resolution) {
  std::vector<DoubleVector> vertices;
  for (size_t i = 0; i < n; ++i) {
    DoubleVector v(n, 1);
    v[i] = 0;
    v[(i+1)%n] = 0;
    vertices.push_back(v);
  }
  DoubleVector center(n, n == 5 ? (std::sqrt(5) - 1) / 2 : 1 / std::sqrt(2));
  std::vector<DoubleVector> parameters;
  parameters.push_back(center);
  for (size_t j = 1; j <= resolution; ++j) {
    double u = (double)j / (double)resolution;
    for (size_t k = 0; k < n; ++k)
      for (size_t i = 0; i < j; ++i) {
        double v = (double)i / (double)j;
        auto ep = affine(vertices[(k+n-1)%n], v, vertices[k]);
        auto p = affine(center, u, ep);
        parameters.push_back(p);
      }
  }
  return parameters;
}

TriMesh triangles(size_t n, size_t resolution) {
  TriMesh mesh;
  mesh.resizePoints(1 + n * resolution * (resolution + 1) / 2);
  size_t inner_start = 0, outer_vert = 1;
  for (size_t layer = 1; layer <= resolution; ++layer) {
    size_t inner_vert = inner_start, outer_start = outer_vert;
    for (size_t side = 0; side < n; ++side) {
      size_t vert = 0;
      while(true) {
        size_t next_vert = (side == n - 1 && vert == layer - 1) ? outer_start : (outer_vert + 1);
        mesh.addTriangle(inner_vert, outer_vert, next_vert);
        ++outer_vert;
        if (++vert == layer)
          break;
        size_t inner_next = (side == n - 1 && vert == layer - 1) ? inner_start : (inner_vert + 1);
        mesh.addTriangle(inner_vert, next_vert, inner_next);
        inner_vert = inner_next;
      }
    }
    inner_start = outer_start;
  }
  return mesh;
}

DoubleVector project(size_t n, const DoubleVector &p) {
  auto f5 = [&](const Powell::Point &x) {
    for (size_t i = 0; i < 5; ++i)
      if (x[i] < 0)
        std::numeric_limits<double>::max();
    double err = 0;
    for (size_t i = 0; i < 5; ++i) // 3 should be enough
      err += std::pow(1 - x[i] - x[(i+2)%n] * x[(i+3)%n], 2);
    return err;
  };
  auto f6 = [&](const Powell::Point &x) {
    for (size_t i = 0; i < 6; ++i)
      if (x[i] < 0)
        std::numeric_limits<double>::max();
    double err = 0;
    for (size_t i = 0; i < 6; ++i) { // 4 should be enough
      double xm = x[(i+n-1)%n], xi = x[i], xp = x[(i+1)%n];
      err += std::pow(xp * xp * (1 - xm * xi) * (1 - 2 * xm * xi) +
                      xp * (2 * xm - 3 * xm * xm * xi + xm * xi * xi) +
                      xm * xm - 1, 2);
    }
    return err;
  };
  Powell::Point x = p;
  for (size_t i = 0; i < 5; ++i)
    if (n == 5)
      Powell::optimize(f5, x, 50, 0.2, 20, 1e-8);
    else
      Powell::optimize(f6, x, 50, 0.2, 20, 1e-8);
  return x;
}

int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <sides> <resolution> [filename]" << std::endl;
    return 1;
  }

  size_t n = std::atoi(argv[1]);
  size_t res = std::atoi(argv[2]);
  std::string filename = std::to_string(n) + "sided.obj";
  if (argc == 4)
    filename = argv[3];

  assert(n == 5 || n == 6);

  auto mesh = triangles(n, res);
  auto verts = vertices(n, res);
  std::ofstream f(filename);
  for (const auto &v : verts) {
    auto p = project(n, v);
    f << 'v';
    for (auto x : p)
      f << ' ' << (x < 0 ? 0 : x);
    f << std::endl;
  }
  for (const auto &t : mesh.triangles())
    f << "f " << t[0] + 1 << ' ' << t[1] + 1 << ' ' << t[2] + 1 << std::endl;
}
