#include "mesh.hpp"

namespace qtgl {

void GLMeshGroup::addIndex3(Index3 idx) {
  addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor, GLMesh::defaultColor);
}

void GLMeshGroup::shade(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
  int n = indices.rows();
  if (parent->normals.rows() == 0 || normIndices.rows() == 0) {  // 无法线信息
    return;
  }
  for (int i = 0; i < n; ++i) {
    Index3 idx = indices.row(i);
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Eigen::Vector3d e0 = (cameraPos.head(3) - p0.head(3)).normalized();
    Eigen::Vector3d e1 = (cameraPos.head(3) - p1.head(3)).normalized();
    Eigen::Vector3d e2 = (cameraPos.head(3) - p2.head(3)).normalized();
    NormIndex normIdx = normIndices.row(i);
    Normal n0 = parent->normals.row(normIdx[0]).normalized();
    Normal n1 = parent->normals.row(normIdx[1]).normalized();
    Normal n2 = parent->normals.row(normIdx[2]).normalized();
    Color01 d0 = colors[i][0];
    Color01 d1 = colors[i][1];
    Color01 d2 = colors[i][2];
    Color01 c0{0, 0, 0, 0}, c1{0, 0, 0, 0}, c2{0, 0, 0, 0};
    for (GLLight* lgt : lights) {
      c0 = c0 + shader->shade(lgt, n0, p0, e0, d0);
      c1 = c1 + shader->shade(lgt, n1, p1, e1, d1);
      c2 = c2 + shader->shade(lgt, n2, p2, e2, d2);
    }
    c0.clamp();
    c1.clamp();
    c2.clamp();
    colors[i][0] = c0;
    colors[i][1] = c1;
    colors[i][2] = c2;
  }
}

void GLMeshGroup::rasterize(Fragments& fragments) {
  int n = indices.rows();
  for (int i = 0; i < n; ++i) {
    Index3 idx = indices.row(i);
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Triangle t(p0, p1, p2);
    std::vector<Color01> clrs = colors[i];
    rasterizeTriangle(t, clrs, fragments);
  }
}

const Color01 GLMesh::defaultColor = {1, 1, 1};
const std::string GLMesh::defaultGroup = "default";

void GLMesh::shade(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
  for (auto g : groups) {
    (g.second)->shade(shader, lights, cameraPos);
  }
}

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
      mesh->pushVertice(strlst[1].toDouble(), strlst[2].toDouble(), strlst[3].toDouble());

    } else if (line[0] == 'f' && line[1] == ' ') {  // face

      QString qline = QString::fromStdString(line);
      QStringList strlst = qline.split(QRegExp("\\s+"));
      std::vector<int> vi;
      std::vector<int> ni;
      for (int i = 1; i < strlst.length(); ++i) {
        QStringList lst = strlst[i].split("/");
        vi.push_back(lst[0].toInt() - 1);
        if (lst.length() >= 2 && !lst[2].isEmpty()) {
          ni.push_back(lst[2].toInt() - 1);
        }
      }
      Index3 idx{vi[0], vi[1], vi[2]};
      mesh->addIndex3(group, idx, Color01::random(), Color01::random(), Color01::random());

      if (!ni.empty()) {
        NormIndex normidx{ni[0], ni[1], ni[2]};
        mesh->addNormIndex(group, normidx);
      }

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
      QString qline = QString::fromStdString(line);
      QStringList strlst = qline.split(QRegExp("\\s+"));
      mesh->pushNormal(strlst[1].toDouble(), strlst[2].toDouble(), strlst[3].toDouble());
    } else if (line[0] == 'v' && line[1] == 't') {  // texture
      // TODO
    } else if (line[0] == '#') {
      ;  // do nothing
    }
  }

  ifs.close();
  return mesh;
}

}  // namespace qtgl