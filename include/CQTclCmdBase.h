#ifndef CQTclCmdBase_H
#define CQTclCmdBase_H

#include <CQTclCmdArgs.h>
#include <QString>
#include <QVariant>
#include <vector>

class CQTclApp;
class CQTclCmdProc;
class CQTcl;

class CQTclCmdBase {
 public:
  using Args = std::vector<QString>;
  using Vars = std::vector<QVariant>;

 public:
  CQTclCmdBase(CQTclApp *app);
 ~CQTclCmdBase();

  void addCommands();

  void addCommand(const QString &name, CQTclCmdProc *proc);

  bool processCmd(const QString &cmd, const Vars &vars);

  //---

  static bool isCompleteLine(QString &line, bool &join);

  void parseLine(const QString &line, bool log=true);

  void parseScriptLine(const QString &line);

  //---

  CQTcl *qtcl() const { return qtcl_; }

 public:
  QStringList stringToCmds(const QString &str) const;

  QStringList stringToCmdData(const QString &str) const;

  QStringList stringToLines(const QString &str) const;

  //---

  bool setCmdRc(int rc);
  bool setCmdRc(double rc);
  bool setCmdRc(const QString &rc);
  bool setCmdRc(const QVariant &rc);
  bool setCmdRc(const QStringList &rc);
  bool setCmdRc(const QVariantList &rc);

  bool setCmdError(const QString &msg);

  //---

 public:
  // widgets
  bool execQtGetWidgetTypesCmd(CQTclCmdArgs &args);
  bool execQtCreateWidgetCmd  (CQTclCmdArgs &args);
  bool execQtGetLayoutTypesCmd(CQTclCmdArgs &args);
  bool execQtCreateLayoutCmd  (CQTclCmdArgs &args);
  bool execQtAddChildWidgetCmd(CQTclCmdArgs &args);
  bool execQtAddStretchCmd    (CQTclCmdArgs &args);
  bool execQtConnectWidgetCmd (CQTclCmdArgs &args);
  bool execQtActivateSlotCmd  (CQTclCmdArgs &args);

  // qt properties
  bool execQtGetPropertyCmd(CQTclCmdArgs &args);
  bool execQtSetPropertyCmd(CQTclCmdArgs &args);
  bool execQtHasPropertyCmd(CQTclCmdArgs &args);

  // misc
  bool execQtSyncCmd(CQTclCmdArgs &args);
  bool execTimerCmd (CQTclCmdArgs &args);

  bool execPerfCmd(CQTclCmdArgs &args);

  bool execAssertCmd(CQTclCmdArgs &args);

  bool execShellCmd(CQTclCmdArgs &args);

  bool execHelpCmd(CQTclCmdArgs &args);

  //---

  bool errorMsg(const QString &msg);

 private:
  using CommandNames = std::vector<QString>;
  using CommandProcs = std::map<QString, CQTclCmdProc *>;

  CQTclApp*    app_          { nullptr };
  bool         continueFlag_ { false };
  CQTcl*       qtcl_         { nullptr };
  CommandNames commandNames_;
  CommandProcs commandProcs_;
};

//---

class CQTclCmd;

class CQTclCmdProc {
 public:
  using Vars = std::vector<QVariant>;

 public:
  CQTclCmdProc(CQTclCmdBase *cmdBase) :
   cmdBase_(cmdBase) {
  }

  virtual ~CQTclCmdProc() { }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  CQTclCmd *tclCmd() const { return tclCmd_; }
  void setTclCmd(CQTclCmd *tclCmd) { tclCmd_ = tclCmd; }

  virtual bool exec(CQTclCmdArgs &args) = 0;

 protected:
  CQTclCmdBase* cmdBase_ { nullptr };
  QString       name_;
  CQTclCmd*     tclCmd_  { nullptr };
};

//---

#define CQTCL_BASE_DEF_CMD(PROC) \
class CQTclCmdBase##PROC##Cmd : public CQTclCmdProc { \
 public: \
  CQTclCmdBase##PROC##Cmd(CQTclCmdBase *cmdBase) : CQTclCmdProc(cmdBase) { } \
 \
  bool exec(CQTclCmdArgs &args) override { return cmdBase_->exec##PROC##Cmd(args); } \
};

#define CQTCL_BASE_ADD_CMD(NAME, PROC) \
addCommand(NAME, new CQTclCmdBase##PROC##Cmd(this))

//---

// widgets
CQTCL_BASE_DEF_CMD(QtGetWidgetTypes)
CQTCL_BASE_DEF_CMD(QtCreateWidget  )
CQTCL_BASE_DEF_CMD(QtGetLayoutTypes)
CQTCL_BASE_DEF_CMD(QtCreateLayout  )
CQTCL_BASE_DEF_CMD(QtAddChildWidget)
CQTCL_BASE_DEF_CMD(QtAddStretch    )
CQTCL_BASE_DEF_CMD(QtConnectWidget )
CQTCL_BASE_DEF_CMD(QtActivateSlot  )

// qt properties
CQTCL_BASE_DEF_CMD(QtGetProperty)
CQTCL_BASE_DEF_CMD(QtSetProperty)
CQTCL_BASE_DEF_CMD(QtHasProperty)

// misc
CQTCL_BASE_DEF_CMD(QtSync)
CQTCL_BASE_DEF_CMD(Timer)

CQTCL_BASE_DEF_CMD(Perf)

CQTCL_BASE_DEF_CMD(Assert)

CQTCL_BASE_DEF_CMD(Shell)

CQTCL_BASE_DEF_CMD(Help)

//---

class CQTclCmdBaseSlot : public QObject {
  Q_OBJECT

 public:
  CQTclCmdBaseSlot(CQTclCmdBase *base, const QString &procName);

  CQTclCmdBase *base() const { return base_; }

  const QString &proc() const { return procName_; }

 public Q_SLOTS:
  void connectSlot();
  void connectSlot(bool);
  void connectSlot(int);
  void connectSlot(double);
  void connectSlot(const QString &);

 private:
  void execProc(const QString &args="");

 private:
  CQTclCmdBase *base_ { nullptr };
  QString       procName_;
};

#endif
