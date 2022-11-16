#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>

#include <geometry.hh>

using namespace Geometry;

using Index = std::vector<size_t>;
using ControlNet = std::map<Index, Point3D>;
using Parameter = DoubleVector;

class ZhengBall {
public:
  static ZhengBall load(std::string filename);
  Point3D eval(const Parameter &u) const;
  TriMesh tessellate(std::string param_filename = "") const;

private:
  size_t n, m;
  ControlNet cnet;
};

static size_t numControlPoints(size_t n, size_t m, bool only_inside) {
  size_t T = m % 2 == 0 ? n * (m - 2) * m / 4 + 1 : n * (m - 1) * (m - 1) / 4;
  return only_inside ? T : T + n * m;
}

ZhengBall ZhengBall::load(std::string filename) {
  ZhengBall result;
  std::ifstream f(filename);
  f.exceptions(std::ios::failbit | std::ios::badbit);
  f >> result.n >> result.m;
  size_t n_cpts = numControlPoints(result.n, result.m, false);
  for (size_t i = 0; i < n_cpts; ++i) {
    Index l(result.n);
    for (size_t j = 0; j < result.n; ++j)
      f >> l[j];
    f >> result.cnet[l];
  }
  return result;
}

static double coefficient(size_t n) {
  switch(n) {
  case 3: return 2.0;
  case 5: return 1.0;
  case 6: return 2.0;
  default: assert(false && "coefficient: only implemented for n=3,5,6");
  }
  return 0;
}

static size_t binomial(size_t n, size_t k) {
  if (k > n)
    return 0;
  size_t result = 1;
  for (size_t d = 1; d <= k; ++d, --n)
    result = result * n / d;
  return result;
}

Point3D ZhengBall::eval(const Parameter &u) const {
  Point3D result(0, 0, 0);
  double B, Bsum = 0;
  for (const auto &[l, p] : cnet) {
    auto it = std::find(l.begin(), l.end(), 0);
    if (it == l.end()) {
      // Inside - (4.5b)
      size_t i = std::min_element(l.begin(), l.end()) - l.begin();
      size_t im = (i + n - 1) % n, ip = (i + 1) % n;
      if (l[im] > l[ip]) {
        im = i;
        i = ip;
      }
      B = binomial(m, l[im]) * binomial(m, l[i]);
      for (size_t j = 0; j < n; ++j)
        B *= std::pow(u[j], l[j]);
    } else {
      // On side i - (4.5a)
      size_t i = it - l.begin();
      size_t im = (i + n - 1) % n, ip = (i + 1) % n;
      size_t k = l[im];
      B = binomial(m, k) * std::pow(u[im], k) * std::pow(u[ip], m - k);
      for (size_t j = 0; j < n; ++j)
        if (j != im && j != i && j != ip)
          B *= std::pow(u[j], m);
      if (n == 3) {
        // Special case - (3.2b)
        // Note: the equation there treats side (i-1), and k and m-k are swapped
        double f = m - k <= k ? m - k + (2 * k - m) * u[ip] : k + (m - 2 * k) * u[im];
        B *= 1 - u[i] * f;
      } else {
        double prod = 1;
        for (size_t j = 0; j < n; ++j)
          prod *= u[j];
        B *= (1 - m * coefficient(n) * prod);
      }
    }
    result += p * B;
    Bsum += B;
  }
  // Correction (2.6)
  double S = Bsum - 1;
  size_t T = numControlPoints(n, m, true);
  for (const auto &[l, p] : cnet)
    if (std::find(l.begin(), l.end(), 0) == l.end())
      result -= p * (S / T);
  return result;
}

static TriMesh loadParameters(std::string filename, std::vector<Parameter> &params) {
  TriMesh result;
  std::ifstream f(filename);
  f.exceptions(std::ios::failbit | std::ios::badbit);
  std::string line;
  std::istringstream ss;
  TriMesh::Triangle t;
  while (!f.eof()) {
    std::getline(f, line);
    f >> std::ws;
    if (line.empty())
      continue;
    switch (line[0]) {
    case 'v':
      if (line[1] != ' ')       // we don't handle vt & vn
        break;
      ss.str(line);
      ss.seekg(2); // skip the first two characters
      {
        double x;
        Parameter p;
        while (ss.rdbuf()->in_avail() > 0) {
          ss >> x;
          p.push_back(x);
        }
        params.push_back(p);
      }
      break;
    case 'f':
      ss.str(line);
      ss.seekg(2); // skip the first two characters
      ss >> t[0];
      ss.ignore(line.size(), ' ');
      ss >> t[1];
      ss.ignore(line.size(), ' ');
      ss >> t[2];
      result.addTriangle(t[0] - 1, t[1] - 1, t[2] - 1);
      break;
    default:
      break;
    }
  }
  result.resizePoints(params.size());
  return result;
}

TriMesh ZhengBall::tessellate(std::string param_filename) const {
  if (param_filename.empty())
    param_filename = std::to_string(n) + "sided.obj";
  std::vector<Parameter> params;
  auto mesh = loadParameters(param_filename, params);
  for (size_t i = 0; i < params.size(); ++i)
    mesh[i] = eval(params[i]);
  return mesh;
}

int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <input.zhb> <output.obj> [parameters.obj]" << std::endl;
    return 1;
  }
  std::string param_filename = "";
  if (argc == 4)
    param_filename = argv[3];

  ZhengBall::load(argv[1]).tessellate(param_filename).writeOBJ(argv[2]);
}
