#include "mesh.hpp"

namespace qtgl {

void GLMeshGroup::addIndex3(Index3 idx) {
  addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor, GLMesh::defaultColor);
}

void GLMeshGroup::rasterize(Fragments& fragments) {
  int n = indices.rows();
  for (int i = 0; i < n; ++i) {
    Index3 idx = indices.row(i);
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Triangle t(p0, p1, p2);
    std::vector<Color> clrs = colors[i];
    rasterizeTriangle(t, clrs, fragments);
  }
}

const Color GLMesh::defaultColor = {255, 255, 255};
const std::string GLMesh::defaultGroup = "default";

void GLMesh::rasterize(Fragments& fragments) {
  for (auto g : groups) {
    (g.second)->rasterize(fragments);
  }
}

GLMesh* GLMesh::readFromObjFile(std::string fpath) {
  GLMesh* mesh;
  std::string group = defaultGroup;
  std::ifstream ifs;
  ifs.open(fpath, std::ios::in);

  if (!ifs.is_open()) {
    return nullptr;
  }
  mesh = new GLMesh();
  char buf[1024] = {0};
  std::string line;

  while (getline(ifs, line)) {
    if (line[0] == 'v' && line[1] == ' ') {  // vertex

      QString qline = QString::fromStdString(line);
      QStringList strlst = qline.split(QRegExp("\\s+"));
      //   std::cout << strlst[1].toFloat() << "," << strlst[2].toFloat() << "," <<
      //   strlst[3].toFloat()
      //             << std::endl;
      mesh->pushVertice(strlst[1].toFloat(), strlst[2].toFloat(), strlst[3].toFloat());

    } else if (line[0] == 'f' && line[1] == ' ') {  // face

      QString qline = QString::fromStdString(line);
      QStringList strlst = qline.split(QRegExp("\\s+"));
      std::vector<int> v;
      for (int i = 1; i < strlst.length(); ++i) {
        v.push_back(strlst[i].split("/")[0].toInt() - 1);
      }
      Index3 idx{v[0], v[1], v[2]};
      //   std::cout << v[0] << "," << v[1] << "," << v[2] << std::endl;
      mesh->addIndex3(group, idx, Color::random(), Color::random(), Color::random());

    } else if (line[0] == 'g') {  // group

      QString qline = QString::fromStdString(line);
      QStringList lst = qline.trimmed().split(QRegExp("\\s+"));
      if (lst.length() < 2) {
        group = defaultGroup;
      } else {
        group = lst[1].toStdString();
      }

      std::cout << group << std::endl;

    } else if (line[0] == 'v' && line[1] == 'n') {  // normal
      // TODO
    } else if (line[0] == 'v' && line[1] == 't') {  // texture
      // TODO
    } else if (line[0] == '#') {
      ;  // do nothing
    }
  }

  return mesh;
}

}  // namespace qtgl