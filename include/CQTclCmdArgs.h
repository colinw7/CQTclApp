#ifndef CQTclCmdArgs_H
#define CQTclCmdArgs_H

#include <CQTclCmdArg.h>
#include <CQTclCmdGroup.h>

#include <CQStrParse.h>

#include <QVariant>
#include <QPolygonF>
#include <QFont>
//#include <boost/optional.hpp>
#include <optional>
#include <iostream>
#include <set>

//------

/*!
 * \brief base class for handling command arguments
 */
class CQTclCmdBaseArgs {
 public:
  using Args    = std::vector<QVariant>;
//using OptReal = boost::optional<double>;
  using OptReal = std::optional<double>;
//using OptInt  = boost::optional<int>;
  using OptInt  = std::optional<int>;
//using OptBool = boost::optional<bool>;
  using OptBool = std::optional<bool>;

  class Arg {
   public:
    Arg(const QVariant &var=QVariant()) :
     var_(var) {
      QString varStr = toString(var_);

      isOpt_ = (varStr.length() && varStr[0] == '-');
    }

  //QString  str() const { assert(! isOpt_); return toString(var_); }
    QVariant var() const { assert(! isOpt_); return var_; }

    bool isOpt() const { return isOpt_; }

    QString opt() const { assert(isOpt_); return toString(var_).mid(1); }

   private:
    QVariant var_;             // arg value
    bool     isOpt_ { false }; // is option
  };

  //---

 public:
  CQTclCmdBaseArgs(const QString &cmdName, const Args &argv) :
   cmdName_(cmdName), argv_(argv), argc_(int(argv_.size())) {
  }

  //---

  // start command group
  CQTclCmdGroup &startCmdGroup(CQTclCmdGroup::Type type) {
    int ind = int(cmdGroups_.size()) + 1;

    cmdGroups_.emplace_back(ind, type);

    groupInd_ = ind;

    return cmdGroups_.back();
  }

  // end command group
  void endCmdGroup() {
    groupInd_ = -1;
  }

  //---

  // add argument
  CQTclCmdArg &addCmdArg(const QString &name, CQTclCmdArg::Type type,
                         const QString &argDesc="", const QString &desc="") {
    int ind = int(cmdArgs_.size()) + 1;

    cmdArgs_.emplace_back(ind, name, type, argDesc, desc);

    CQTclCmdArg &cmdArg = cmdArgs_.back();

    cmdArg.setGroupInd(groupInd_);

    return cmdArg;
  }

  //---

  // has processed last argument
  bool eof() const { return (i_ >= argc_); }

  // move to first argument
  void rewind() { i_ = 0; }

  const Arg &getArg() {
    assert(i_ < argc_);

    lastArg_ = Arg(argv_[size_t(i_++)]);

    return lastArg_;
  }

  //---

  // get string or string list value of current option
  bool getOptValue(QStringList &strs) {
    if (eof()) return false;

    strs = toStringList(argv_[size_t(i_++)]);

    return true;
  }

  // get string value of current option
  bool getOptValue(QString &str) {
    if (eof()) return false;

    str = toString(argv_[size_t(i_++)]);

    return true;
  }

  // get integer value of current option
  bool getOptValue(int &i) {
    QString str;

    if (! getOptValue(str))
      return false;

    bool ok;

    i = toInt(str, ok);

    return ok;
  }

  // get real value of current option
  bool getOptValue(double &r) {
    QString str;

    if (! getOptValue(str))
      return false;

    bool ok;

    r = toReal(str, ok);

    return ok;
  }

  // get optional real value of current option
  bool getOptValue(OptReal &r) {
    QString str;

    if (! getOptValue(str))
      return false;

    bool ok;

    r = toReal(str, ok);

    return ok;
  }

  // get optional boolean value of current option
  bool getOptValue(bool &b) {
    QString str;

    if (! getOptValue(str))
      return false;

    str = str.toLower();

    bool ok;

    b = stringToBool(str, &ok);

    return ok;
  }

  // get generic value of current option
  template<typename T>
  bool getOptValue(T &t) {
    QString str;

    if (! getOptValue(str))
      return false;

    t = T(str);

    return true;
  }

  // get font value of current option
  bool getOptValue(QFont &f) {
    QString str;

    if (! getOptValue(str))
      return false;

    f = QFont(str);

    return true;
  }

  // get polygon value of current option
  bool getOptValue(QPolygonF &poly) {
    QString str;

    if (! getOptValue(str))
      return false;

    poly = stringToPolygon(str);

    return poly.length();
  }

  //---

  // get/set debug
  bool isDebug() const { return debug_; }
  void setDebug(bool b) { debug_ = b; }

  //---

  // parse command arguments
  bool parse() {
    bool rc;

    return parse(rc);
  }

  bool parse(bool &rc) {
    rc = false;

    // clear parsed values
    parseInt_ .clear();
    parseReal_.clear();
    parseStr_ .clear();
    parseBool_.clear();
    parseArgs_.clear();

    //---

    using Names      = std::set<QString>;
    using GroupNames = std::map<int, Names>;

    GroupNames groupNames;

    //---

    bool help = false;

    while (! eof()) {
      // get next arg
      const Arg &arg = getArg();

      // handle option (starts with '-')
      if (arg.isOpt()) {
        // get option name
        QString opt = arg.opt();

        //---

        // flag if help option
        if (opt == "help") {
          help = true;
          continue;
        }

        // TODO: handle '--' for no more options

        //---

        // get arg data for option (flag error if not found)
        CQTclCmdArg *cmdArg = getCmdOpt(opt);

        if (! cmdArg) {
          return this->error();
        }

        //---

        // record option for associated group (if any)
        if (cmdArg->groupInd() >= 0) {
          groupNames[cmdArg->groupInd()].insert(cmdArg->name());
        }

        //---

        // handle bool option (no value)
        if      (cmdArg->type() == CQTclCmdArg::Type::Boolean) {
          parseBool_[opt] = true;
        }
        // handle integer option
        else if (cmdArg->type() == CQTclCmdArg::Type::Integer) {
          int i = 0;

          if (getOptValue(i)) {
            parseInt_[opt] = i;
          }
          else {
            return valueError(opt);
          }
        }
        // handle real option
        else if (cmdArg->type() == CQTclCmdArg::Type::Real) {
          double r = 0.0;

          if (getOptValue(r)) {
            parseReal_[opt] = r;
          }
          else {
            return valueError(opt);
          }
        }
        // handle string option
        else if (cmdArg->type() == CQTclCmdArg::Type::String) {
          if (cmdArg->isMultiple()) {
            QStringList strs;

            if (getOptValue(strs)) {
              for (int i = 0; i < strs.length(); ++i)
                parseStr_[opt].push_back(strs[i]);
            }
            else
              return valueError(opt);
          }
          else {
            QString str;

            if (getOptValue(str))
              parseStr_[opt].push_back(str);
            else
              return valueError(opt);
          }
        }
        // handle string bool option
        else if (cmdArg->type() == CQTclCmdArg::Type::SBool) {
          bool b;

          if (getOptValue(b)) {
            parseBool_[opt] = b;
          }
          else {
            return valueError(opt);
          }
        }
        // handle enum option (string)
        else if (cmdArg->type() == CQTclCmdArg::Type::Enum) {
          QString str;

          if (getOptValue(str)) {
            bool found = false;

            for (auto &nv : cmdArg->nameValues()) {
              if (str == nv.first) {
                parseInt_[opt] = nv.second;
                found = true;
                break;
              }
            }

            if (! found) {
              QString msg =
                QString("Invalid value '%1' for '-%2' should be one of :\n ").arg(str).arg(opt);
              for (auto &nv : cmdArg->nameValues())
                msg += " " + nv.first;
              errorMsg(msg);
              return false;
            }
          }
          else {
            return valueError(opt);
          }
        }
        // handle color option (string)
        else if (cmdArg->type() == CQTclCmdArg::Type::Color) {
          QString str;

          if (getOptValue(str)) {
            parseStr_[opt].push_back(str);
          }
          else {
            return valueError(opt);
          }
        }
        // handle position option (string)
        else if (cmdArg->type() == CQTclCmdArg::Type::Position) {
          QString str;

          if (getOptValue(str)) {
            parseStr_[opt].push_back(str);
          }
          else {
            return valueError(opt);
          }
        }
        // handle rect option (string)
        else if (cmdArg->type() == CQTclCmdArg::Type::Rect) {
          QString str;

          if (getOptValue(str)) {
            parseStr_[opt].push_back(str);
          }
          else {
            return valueError(opt);
          }
        }
        // handle polygon option (string)
        else if (cmdArg->type() == CQTclCmdArg::Type::Polygon) {
          QString str;

          if (getOptValue(str)) {
            parseStr_[opt].push_back(str);
          }
          else {
            return valueError(opt);
          }
        }
        // invalid type (assert ?)
        else {
          std::cerr << "Invalid type for '" << opt.toStdString() << "'\n";
          continue;
        }
      }
      // handle argument (no '-')
      else {
        // save argument
        parseArgs_.push_back(arg.var());
      }
    }

    //---

    // if help option specified ignore other options and process help
    if (help) {
      this->help();

      if (! isDebug()) {
        rc = true;

        return false;
      }
    }

    //---

    // check options specified for cmd groups
    for (const auto &cmdGroup : cmdGroups_) {
      int groupInd = cmdGroup.ind();

      auto p = groupNames.find(groupInd);

      // handle no options for required group
      if (p == groupNames.end()) {
        if (cmdGroup.isRequired()) {
          std::string names = getGroupArgNames(groupInd).join(", ").toStdString();
          std::cerr << "One of " << names << " required\n";
          return false;
        }
      }
      // handle multiple options for one of group
      else {
        const Names &names = (*p).second;

        if (names.size() > 1) {
          std::string names = getGroupArgNames(groupInd).join(", ").toStdString();
          std::cerr << "Only one of " << names << " allowed\n";
          return false;
        }
      }
    }

    //---

    //  display parsed data for debug
    if (isDebug()) {
      for (auto &pi : parseInt_) {
        std::cerr << pi.first.toStdString() << "=" << pi.second << "\n";
      }
      for (auto &pr : parseReal_) {
        std::cerr << pr.first.toStdString() << "=" << pr.second << "\n";
      }
      for (auto &ps : parseStr_) {
        const QString     &name   = ps.first;
        const QStringList &values = ps.second;

        for (int i = 0; i < ps.second.length(); ++i) {
          std::cerr << name.toStdString() << "=" << values[i].toStdString() << "\n";
        }
      }
      for (auto &ps : parseBool_) {
        std::cerr << ps.first.toStdString() << "=" << ps.second << "\n";
      }
      for (auto &a : parseArgs_) {
        std::cerr << toString(a).toStdString() << "\n";
      }
    }

    //---

    rc = true;

    return true;
  }

  //---

  // check if option found by parse
  bool hasParseArg(const QString &name) const {
    if      (parseInt_ .find(name) != parseInt_ .end()) return true;
    else if (parseReal_.find(name) != parseReal_.end()) return true;
    else if (parseStr_ .find(name) != parseStr_ .end()) return true;
    else if (parseBool_.find(name) != parseBool_.end()) return true;

    return false;
  }

  //---

  // get parsed int for option (default returned if not found)
  int getParseInt(const QString &name, int def=0) const {
    auto p = parseInt_.find(name);
    if (p == parseInt_.end()) return def;

    return (*p).second;
  }

  // get parsed optional int for option
  OptInt getParseOptInt(const QString &name) const {
    auto p = parseInt_.find(name);
    if (p == parseInt_.end()) return OptInt();

    return OptInt((*p).second);
  }

  // get parsed real for option (default returned if not found)
  double getParseReal(const QString &name, double def=0.0) const {
    auto p = parseReal_.find(name);
    if (p == parseReal_.end()) return def;

    return (*p).second;
  }

  // get parsed optional real for option
  OptReal getParseOptReal(const QString &name) const {
    auto p = parseReal_.find(name);
    if (p == parseReal_.end()) return OptReal();

    return OptReal((*p).second);
  }

  // get number of parsed strings for option
  int getNumParseStrs(const QString &name) const {
    auto p = parseStr_.find(name);
    if (p == parseStr_.end()) return 0;

    return int((*p).second.size());
  }

  // get parsed strings for option
  QStringList getParseStrs(const QString &name) const {
    auto p = parseStr_.find(name);
    if (p == parseStr_.end()) return QStringList();

    return (*p).second;
  }

  // get parsed string for option (if multiple return first) (default returned if not found)
  QString getParseStr(const QString &name, const QString &def="") const {
    auto p = parseStr_.find(name);
    if (p == parseStr_.end()) return def;

    return (*p).second[0];
  }

  // get parsed boolean for option (default returned if not found)
  bool getParseBool(const QString &name, bool def=false) const {
    auto p = parseBool_.find(name);
    if (p == parseBool_.end()) return def;

    return (*p).second;
  }

  // get parsed optional boolean for option
  OptBool getParseOptBool(const QString &name) const {
    auto p = parseBool_.find(name);
    if (p == parseBool_.end()) return OptBool();

    return OptBool((*p).second);
  }

  // get parsed polygon value for option
  QPolygonF getParsePoly(const QString &name, const QPolygonF &def=QPolygonF()) const {
    auto p = parseStr_.find(name);
    if (p == parseStr_.end()) return def;

    QString value = (*p).second[0];

    QPolygonF poly;

    if (! parsePoly(value, poly))
      return def;

    return poly;
  }

  // get parsed generic value for option
  template<typename T>
  T getParseValue(const QString &name, const T &def=T()) const {
    auto p = parseStr_.find(name);
    if (p == parseStr_.end()) return def;

    return T((*p).second[0]);
  }

  // get parsed font for option (default returned if not found)
  QFont getParseFont(const QString &name, const QFont &def=QFont()) const {
    return getParseValue<QFont>(name, def);
  }

  //---

  // get parsed args (non options)
  const Args &getParseArgs() const { return parseArgs_; }

  //---

  // get arg data for option
  CQTclCmdArg *getCmdOpt(const QString &name) {
    for (auto &cmdArg : cmdArgs_) {
      if (cmdArg.isOpt() && cmdArg.name() == name)
        return &cmdArg;
    }

    return nullptr;
  }

  //---

  // handle missing value error
  bool valueError(const QString &opt) {
    errorMsg(QString("Missing value for '-%1'").arg(opt));
    return false;
  }

  // handle invalid option/arg error
  bool error() {
    if (lastArg_.isOpt())
      errorMsg("Invalid option '" + lastArg_.opt() + "'");
    else
      errorMsg("Invalid arg '" + toString(lastArg_.var()) + "'");
    return false;
  }

  //---

  // display help
  void help() const {
    using GroupIds = std::set<int>;

    GroupIds groupInds;

    std::cerr << cmdName_.toStdString() << "\n";

    for (auto &cmdArg : cmdArgs_) {
      int groupInd = cmdArg.groupInd();

      if (groupInd > 0) {
        auto p = groupInds.find(groupInd);

        if (p == groupInds.end()) {
          std::cerr << "  ";

          helpGroup(groupInd);

          std::cerr << "\n";

          groupInds.insert(groupInd);
        }
        else
          continue;
      }
      else {
        std::cerr << "  ";

        if (! cmdArg.isRequired())
         std::cerr << "[";

        helpArg(cmdArg);

        if (! cmdArg.isRequired())
          std::cerr << "]";

        std::cerr << "\n";
      }
    }

    std::cerr << "  [-help]\n";
  }

  // display help for group
  void helpGroup(int groupInd) const {
    const CQTclCmdGroup &cmdGroup = cmdGroups_[size_t(groupInd - 1)];

    if (! cmdGroup.isRequired())
      std::cerr << "[";

    CmdArgs cmdArgs;

    getGroupCmdArgs(groupInd, cmdArgs);

    int i = 0;

    for (const auto &cmdArg : cmdArgs) {
      if (i > 0)
        std::cerr << "|";

      helpArg(cmdArg);

      ++i;
    }

    if (! cmdGroup.isRequired())
      std::cerr << "]";
  }

  // display help for arg
  void helpArg(const CQTclCmdArg &cmdArg) const {
    if (cmdArg.isOpt()) {
      std::cerr << "-" << cmdArg.name().toStdString() << " ";

      if (cmdArg.type() != CQTclCmdArg::Type::Boolean)
        std::cerr << "<" << cmdArg.argDesc().toStdString() << ">";
    }
    else {
      std::cerr << "<" << cmdArg.argDesc().toStdString() << ">";
    }
  }

  //---

  // variant to string
  static QStringList toStringList(const QVariant &var) {
    QStringList strs;

    if (var.type() == QVariant::List) {
      QList<QVariant> vars = var.toList();

      for (int i = 0; i < vars.length(); ++i) {
        QString str = toString(vars[i]);

        strs.push_back(str);
      }
    }
    else {
      strs.push_back(var.toString());
    }

    return strs;
  }

  //---

  // variant to string
  static QString toString(const QVariant &var) {
    if (var.type() == QVariant::List) {
      QList<QVariant> vars = var.toList();

      QStringList strs;

      for (int i = 0; i < vars.length(); ++i) {
        QString str = toString(vars[i]);

        strs.push_back(str);
      }

      return "{" + strs.join(" ") + "}";
    }
    else
      return var.toString();
  }

  //---

  // variant to bool
  static bool stringToBool(const QString &str, bool *ok) {
    QString lstr = str.toLower();

    if (lstr == "0" || lstr == "false" || lstr == "no") {
      *ok = true;
      return false;
    }

    if (lstr == "1" || lstr == "true" || lstr == "yes") {
      *ok = true;
      return true;
    }

    *ok = false;

    return false;
  }

 private:
  // string to polygon
  QPolygonF stringToPolygon(const QString &str) const {
    QPolygonF poly;

    CQStrParse parse(str);

    while (! parse.eof()) {
      parse.skipSpace();

      QString xstr;

      if (! parse.readNonSpace(xstr))
        break;

      parse.skipSpace();

      QString ystr;

      if (! parse.readNonSpace(ystr))
        break;

      parse.skipSpace();

      double x, y;

      if (! toReal(xstr, x))
        break;

      if (! toReal(ystr, y))
        break;

      QPointF p(x, y);

      poly << p;
    }

    return poly;
  }

  // parse polygon from string
  bool parsePoly(const QString &str, QPolygonF &poly) const {
    CQStrParse parse(str);

    return parsePoly(parse, poly);
  }

  // parse polygon at parse position
  bool parsePoly(CQStrParse &parse, QPolygonF &poly) const {
    parse.skipSpace();

    if (parse.isChar('{')) {
      int pos1 = parse.getPos();

      parse.skipChar();

      parse.skipSpace();

      if (parse.isChar('{')) {
        parse.setPos(pos1);

        if (! parse.skipBracedString())
          return false;

        int pos2 = parse.getPos();

        QString str = parse.getAt(pos1 + 1, pos2 - pos1 - 2);

        return parsePoly(str, poly);
      }

      parse.setPos(pos1);
    }

    //--

    while (! parse.eof()) {
      QPointF p;

      if (! parsePoint(parse, p))
        return false;

      poly << p;
    }

    return true;
  }

  // parse point from string
  bool parsePoint(const QString &str, QPointF &pos) const {
    CQStrParse parse(str);

    return parsePoint(parse, pos);
  }

  // parse point at parse position
  bool parsePoint(CQStrParse &parse, QPointF &pos) const {
    parse.skipSpace();

    if (parse.isChar('{')) {
      int pos1 = parse.getPos();

      if (! parse.skipBracedString())
        return false;

      int pos2 = parse.getPos();

      QString str = parse.getAt(pos1 + 1, pos2 - pos1 - 2);

      return parsePoint(str, pos);
    }

    QString xstr;

    if (! parse.readNonSpace(xstr))
      return false;

    parse.skipSpace();

    QString ystr;

    if (! parse.readNonSpace(ystr))
      return false;

    parse.skipSpace();

    double x, y;

    if (! toReal(xstr, x))
      return false;

    if (! toReal(ystr, y))
      return false;

    pos = QPointF(x, y);

    return true;
  }

  int toInt(const QString &str, bool &ok) const {
    return str.toInt(&ok);
  }

  double toReal(const QString &str, bool &ok) const {
    return str.toDouble(&ok);
  }

  bool toReal(const QString &str, double &r) const {
    bool ok;

    r = str.toDouble(&ok);

    return ok;
  }

 private:
  using CmdArgs     = std::vector<CQTclCmdArg>;
  using CmdGroups   = std::vector<CQTclCmdGroup>;
  using NameInt     = std::map<QString, int>;
  using NameReal    = std::map<QString, double>;
  using NameString  = std::map<QString, QString>;
  using NameStrings = std::map<QString, QStringList>;
  using NameBool    = std::map<QString, bool>;

 private:
  // get option names for group
  QStringList getGroupArgNames(int groupInd) const {
    CmdArgs cmdArgs;

    getGroupCmdArgs(groupInd, cmdArgs);

    QStringList names;

    for (const auto &cmdArg : cmdArgs)
      names << cmdArg.name();

    return names;
  }

  // get option datas for group
  void getGroupCmdArgs(int groupInd, CmdArgs &cmdArgs) const {
    for (auto &cmdArg : cmdArgs_) {
      int groupInd1 = cmdArg.groupInd();

      if (groupInd1 != groupInd)
        continue;

      cmdArgs.push_back(cmdArg);
    }
  }

 private:
  // display error message
  void errorMsg(const QString &msg) {
    std::cerr << msg.toStdString() << "\n";
  }

 protected:
  QString     cmdName_;              //! command name being processed
  bool        debug_      { false }; //! is debug
  Args        argv_;                 //! input args
  int         i_          { 0 };     //! current arg
  int         argc_       { 0 };     //! number of args
  Arg         lastArg_;              //! last processed arg
  CmdArgs     cmdArgs_;              //! command argument data
  CmdGroups   cmdGroups_;            //! command argument groups
  int         groupInd_   { -1 };    //! current group index
  NameInt     parseInt_;             //! parsed option integers
  NameReal    parseReal_;            //! parsed option reals
  NameStrings parseStr_;             //! parsed option strings
  NameBool    parseBool_;            //! parsed option booleans
  Args        parseArgs_;            //! parsed arguments
};

//------

/*!
 * \brief derived class for handling command arguments
 */
class CQTclCmdArgs : public CQTclCmdBaseArgs {
 public:
  CQTclCmdArgs(const QString &cmdName, const Args &argv) :
   CQTclCmdBaseArgs(cmdName, argv) {
  }
};

#endif
