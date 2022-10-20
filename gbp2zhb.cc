#include <fstream>

#include <geometry.hh>

using namespace Geometry;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input.gbp> <output.zhb>" << std::endl;
    return 1;
  }

  std::ifstream fin(argv[1]);
  fin.exceptions(std::ios::failbit | std::ios::badbit);
  std::ofstream fout(argv[2]);

  size_t n, d;
  fin >> n >> d;
  fout << n << ' ' << d << std::endl;
  Point3D center;
  fin >> center[0] >> center[1] >> center[2];
  if (d % 2 == 0) {
    // There is no center point when the degree is odd
    for (size_t i = 0; i < n; ++i)
      fout << d / 2 << ' ';
    fout << center << std::endl;
  }

  size_t l = (d + 1) / 2;
  size_t cp = 1 + d / 2;
  cp = n * cp * l + 1;          // # of control points
  Point3D p;
  for (size_t i = 1, side = 0, col = 0, row = 0; i < cp; ++i, ++col) {
    if (col >= d - row) {
      if (++side >= n) {
        side = 0;
        ++row;
      }
      col = row;
    }
    fin >> p[0] >> p[1] >> p[2];
    size_t a = side, La = row, b = (side + n - 1) % n, Lb = col;
    if (col > d / 2) {
      b = a;
      Lb = La;
      a = (side + 1) % n;
      La = d - col;
    }
    std::vector<size_t> L(n, d - std::min(La, Lb));
    L[a] = La;
    L[b] = Lb;
    L[(a+1)%n] = d - Lb;
    L[(b+n-1)%n] = d - La;
    for (size_t j = 0; j < n; ++j)
      fout << L[j] << ' ';
    fout << p << std::endl;
  }
}
