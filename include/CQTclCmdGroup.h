#ifndef CQTclCmdGroup_H
#define CQTclCmdGroup_H

class CQTclCmdGroup {
 public:
  enum class Type {
    None,
    OneOpt,
    OneReq
  };

 public:
  CQTclCmdGroup(int ind, Type type) :
   ind_(ind), type_(type) {
  }

  int ind() const { return ind_; }

  bool isRequired() const { return (type_ == Type::OneReq); }

 private:
  int  ind_  { -1 };
  Type type_ { Type::None };
};

#endif
