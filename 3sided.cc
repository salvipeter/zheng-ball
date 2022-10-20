#include <cmath>
#include <iostream>

#include <geometry.hh> // https://github.com/salvipeter/libgeom/
#include <powell.hh>   // https://github.com/salvipeter/dfo/

using namespace Geometry;

PointVector vertices(size_t resolution) {
  PointVector vertices = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
  // Center at (x,x,x) with x = (sqrt(3) - 1) / 2
  PointVector parameters;
  for (size_t j = 0; j <= resolution; ++j) {
    double u = (double)j / resolution;
    auto p = vertices[0] * u + vertices[2] * (1 - u);
    auto q = vertices[1] * u + vertices[2] * (1 - u);
    for (size_t k = 0; k <= j; ++k) {
      double v = j == 0 ? 1.0 : (double)k / j;
      parameters.push_back(p * (1 - v) + q * v);
    }
  }
  return parameters;
}

TriMesh triangles(size_t resolution) {
  TriMesh mesh;
  mesh.resizePoints((resolution + 1) * (resolution + 2) / 2);
  size_t prev = 0, current = 1;
  for (size_t i = 0; i < resolution; ++i) {
    for (size_t j = 0; j < i; ++j) {
      mesh.addTriangle(current + j, current + j + 1, prev + j);
      mesh.addTriangle(current + j + 1, prev + j + 1, prev + j);
    }
    mesh.addTriangle(current + i, current + i + 1, prev + i);
    prev = current;
    current += i + 2;
  }
  return mesh;
}

Point3D project(const Point3D &p) {
  auto f = [&](const Powell::Point &x) {
    if (x[0] < 0 || x[1] < 0 || x[2] < 0)
      std::numeric_limits<double>::max();
    return std::pow(x[0] + x[1] + x[2] - 2 * x[0] * x[1] * x[2] - 1, 2);
  };
  Powell::Point x = { p[0], p[1], p[2] };
  for (size_t i = 0; i < 5; ++i)
    Powell::optimize(f, x, 50, 0.2, 20, 1e-8);
  for (size_t i = 0; i < 3; ++i)
    if (x[i] < 0)
      x[i] = 0;
  return { x[0], x[1], x[2] };
}

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <resolution> [filename]" << std::endl;
    return 1;
  }

  std::string filename = "3sided.obj";
  size_t res = std::atoi(argv[1]);
  if (argc == 3)
    filename = argv[2];

  auto mesh = triangles(res);
  auto verts = vertices(res);
  for (size_t i = 0; i < verts.size(); ++i)
    mesh[i] = project(verts[i]);
  mesh.writeOBJ(filename);
}
