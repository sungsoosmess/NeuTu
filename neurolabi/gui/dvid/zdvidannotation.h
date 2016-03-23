#ifndef ZDVIDANNOTATION_H
#define ZDVIDANNOTATION_H

#include <string>
#include "zstackobject.h"
#include "zjsonobject.h"
#include "zjsonarray.h"

class ZJsonObject;
class ZCuboid;

class ZDvidAnnotation : public ZStackObject
{
public:
  ZDvidAnnotation();

  enum EKind { KIND_POST_SYN, KIND_PRE_SYN, KIND_NOTE, KIND_UNKNOWN,
               KIND_INVALID };

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_ANNOTATION;
  }

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  void setPosition(int x, int y, int z);
  void setPosition(const ZIntPoint &pos);

  const ZIntPoint& getPosition() const { return m_position; }

  void setDefaultRadius();
  void setRadius(double r) { m_radius = r; }

  double getRadius() const { return m_radius; }

  void setKind(EKind kind) { m_kind = kind; }
  EKind getKind() const { return m_kind; }
  static std::string GetKindName(EKind kind);
  static EKind GetKind(const std::string &name);

  void setKind(const std::string &kind);

  uint64_t getBodyId() const {
    return m_bodyId;
  }

  void setBodyId(uint64_t bodyId) {
    m_bodyId = bodyId;
  }

  void setDefaultColor();

  int getX() const;
  int getY() const;
  int getZ() const;

  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y, NeuTube::EAxis axis);
  bool hit(double x, double y, double z);

  void loadJsonObject(
      const ZJsonObject &obj,
      NeuTube::FlyEM::EDvidAnnotationLoadMode mode);
  ZJsonObject toJsonObject() const;

  void clearPartner();
  void addPartner(int x, int y, int z);
  void addTag(const std::string &tag);

  void clear();

  bool isValid() const;

  static QColor GetDefaultColor(EKind kind);
  static double GetDefaultRadius(EKind kind);

  class Relation {
  public:
    enum ERelation {
      RELATION_UNKNOWN, RELATION_POSTSYN_TO, RELATION_PRESYN_TO,
      RELATION_CONVERGENT_TO, RELATION_GROUPED_WITH
    };

    Relation(const ZIntPoint &to, ERelation relation) :
      m_to(to), m_relation(relation) {
    }

    static std::string GetName(ERelation rel);

    ZJsonObject toJsonObject() const;

  private:
    ZIntPoint m_to;
    ERelation m_relation;
  };

  const std::vector<ZIntPoint>& getPartners() {
    return m_partnerHint;
  }

  ZCuboid getBoundBox() const;

public: //Additional properties
  void setUserName(const std::string &name);
  std::string getUserName() const;

public: //Json APIs
  static ZJsonObject MakeRelJson(const ZIntPoint &pt, const std::string &rel);
  static bool AddRelation(
      ZJsonObject &json, const ZIntPoint &to, const std::string &rel);
  static bool AddRelation(
      ZJsonArray &json, const ZIntPoint &to, const std::string &rel);
  static bool AddRelation(ZJsonObject &json, const ZJsonObject &relJson);
  static bool AddRelation(ZJsonArray &json, const ZJsonObject &relJson);
  static int AddRelation(ZJsonObject &json, const ZJsonArray &relJson);
  static int AddRelation(ZJsonArray &json, const ZJsonArray &relJson);
  template <typename InputIterator>
  static int AddRelation(
      ZJsonObject &json, const InputIterator &first,
      const InputIterator &last, const std::string &rel);
   static ZJsonArray GetRelationJson(ZJsonObject &json);

  static bool RemoveRelation(ZJsonArray &json, const ZIntPoint &pt);
  static bool RemoveRelation(ZJsonObject &json, const ZIntPoint &pt);

  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const std::string &value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          bool value);

protected:
  bool isSliceVisible(int z, NeuTube::EAxis sliceAxis) const;
  double getRadius(int z, NeuTube::EAxis sliceAxis) const;

private:
  void init();

protected:
  ZIntPoint m_position;
  EKind m_kind;
  double m_radius;
  uint64_t m_bodyId;

  std::vector<ZIntPoint> m_partnerHint;
  std::vector<std::string> m_tagArray;
  ZJsonObject m_propertyJson;
  ZJsonArray m_relJson;
};

template <typename InputIterator>
int ZDvidAnnotation::AddRelation(
    ZJsonObject &json, const InputIterator &first,
    const InputIterator &last, const std::string &rel)
{
  int count = 0;
  for (InputIterator iter = first; iter != last; ++iter) {
    if (AddRelation(json, *iter, rel)) {
      ++count;
    }
  }

  return count;
}


#endif // ZDVIDANNOTATION_H