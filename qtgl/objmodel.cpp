#include "objmodel.hpp"

namespace qtgl {

const std::string ObjModel::defaultGroup = "default";

ObjModel* ObjModel::loadObj(const std::string& objpath) {
  std::string group = ObjModel::defaultGroup;
  std::ifstream ifs;
  ifs.open(objpath, std::ios::in);

  if (!ifs.is_open()) {
    return nullptr;
  }
  ObjModel* model = new ObjModel;
  std::string line;
  int i = 0;

  while (getline(ifs, line)) {
    if (i++ % 1000 == 0) {
      std::cout << i << std::endl;
    }
    QString qline = QString::fromStdString(line);
    if (qline.startsWith("v ")) {  // vertex
      QStringList strlst = qline.split(QRegExp("\\s+"));
      model->pushVertice(strlst[1].toDouble(), strlst[2].toDouble(), strlst[3].toDouble());
    } else if (qline.startsWith("f ")) {  // face
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
      model->addIndex3(group, idx);
      if (!ni.empty()) {
        NormIndex normidx{ni[0], ni[1], ni[2]};
        model->addNormIndex(group, normidx);
      }
    } else if (qline.startsWith("g ")) {  // group
      QStringList lst = qline.trimmed().split(QRegExp("\\s+"));
      if (lst.length() < 2) {
        group = defaultGroup;
      } else {
        group = lst[1].toStdString();
      }

      std::cout << group << std::endl;

    } else if (qline.startsWith("vn")) {  // normal
      QString qline = QString::fromStdString(line);
      QStringList strlst = qline.split(QRegExp("\\s+"));
      model->pushNormal(strlst[1].toDouble(), strlst[2].toDouble(), strlst[3].toDouble());
    } else if (qline.startsWith("vt")) {  // texture
      // TODO
    } else if (qline.startsWith("#")) {
      ;  // do nothing
    } else if (qline.startsWith("mtllib")) {
    } else if (qline.startsWith("o ")) {
    } else if (qline.startsWith("s")) {
    } else if (qline.startsWith("usemtl")) {
    }
  }

  ifs.close();
  return model;
}
}  // namespace qtgl