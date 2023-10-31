#include <CQTclCmdBase.h>
#include <CQTclApp.h>
#include <CQTclControl.h>
#include <CQTclUtil.h>

#include <CQWidgetFactory.h>
#include <CQFlowLayout.h>
#include <CQPerfMonitor.h>
#include <CQUtil.h>
#include <CQStrParse.h>
#include <CEnv.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeWidget>

#include <QGridLayout>
#include <QFormLayout>

namespace {

static bool s_slots_enabled = true;

}

//---

class CQTclCmd {
 public:
  using Vars = std::vector<QVariant>;

 public:
  CQTclCmd(CQTclCmdBase *cmdBase, const QString &name) :
   cmdBase_(cmdBase), name_(name) {
    cmdId_ = cmdBase_->qtcl()->createObjCommand(name_,
               reinterpret_cast<CQTcl::ObjCmdProc>(&CQTclCmd::commandProc),
               static_cast<CQTcl::ObjCmdData>(this));
  }

  static int commandProc(ClientData clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
    auto *command = static_cast<CQTclCmd *>(clientData);

    Vars vars;

    for (int i = 1; i < objc; ++i) {
      auto *obj = const_cast<Tcl_Obj *>(objv[i]);

      vars.push_back(command->cmdBase_->qtcl()->variantFromObj(obj));
    }

    if (! command->cmdBase_->processCmd(command->name_, vars))
      return TCL_ERROR;

    return TCL_OK;
  }

 private:
  CQTclCmdBase *cmdBase_ { nullptr };
  QString       name_;
  Tcl_Command   cmdId_   { nullptr };
};

//----

CQTclCmdBase::
CQTclCmdBase(CQTclApp *app) :
 app_(app)
{
  qtcl_ = new CQTcl();
}

CQTclCmdBase::
~CQTclCmdBase()
{
  delete qtcl_;
}

void
CQTclCmdBase::
addCommands()
{
  static bool cmdsAdded;

  if (! cmdsAdded) {
    auto wmgr = CQWidgetFactoryMgrInst;

    // containers (menu ?)
    wmgr->addWidgetFactoryT<QFrame        >("gui:frame");
    wmgr->addWidgetFactoryT<QGroupBox     >("gui:group");
    wmgr->addWidgetFactoryT<QSplitter     >("gui:splitter");
    wmgr->addWidgetFactoryT<QTabWidget    >("gui:tab");
    wmgr->addWidgetFactoryT<QStackedWidget>("gui:stack");

    // controls
    wmgr->addWidgetFactoryT<QCheckBox     >("gui:check");
    wmgr->addWidgetFactoryT<QComboBox     >("gui:combo");
    wmgr->addWidgetFactoryT<QDoubleSpinBox>("gui:double_spinbox");
    wmgr->addWidgetFactoryT<QLabel        >("gui:label");
    wmgr->addWidgetFactoryT<QLineEdit     >("gui:line_edit");
    wmgr->addWidgetFactoryT<QListWidget   >("gui:list");
    wmgr->addWidgetFactoryT<QPushButton   >("gui:push_button");
    wmgr->addWidgetFactoryT<QRadioButton  >("gui:radio_button");
    wmgr->addWidgetFactoryT<QSpinBox      >("gui:spinbox");
    wmgr->addWidgetFactoryT<QTableWidget  >("gui:table");
    wmgr->addWidgetFactoryT<QTextBrowser  >("gui:text_browser");
    wmgr->addWidgetFactoryT<QTextEdit     >("gui:text_edit");
    wmgr->addWidgetFactoryT<QToolButton   >("gui:tool_button");
    wmgr->addWidgetFactoryT<QTreeWidget   >("gui:tree");

    // layouts
    wmgr->addLayoutFactoryT<QHBoxLayout >("gui:hbox");
    wmgr->addLayoutFactoryT<QVBoxLayout >("gui:vbox");
    wmgr->addLayoutFactoryT<QGridLayout >("gui:grid");
    wmgr->addLayoutFactoryT<QFormLayout >("gui:form");
    wmgr->addLayoutFactoryT<CQFlowLayout>("gui:flow");

    //---

    CQTCL_BASE_ADD_CMD("help", Help);

    // qt generic
    CQTCL_BASE_ADD_CMD("qt_get_widget_types", QtGetWidgetTypes);
    CQTCL_BASE_ADD_CMD("qt_create_widget"   , QtCreateWidget);
    CQTCL_BASE_ADD_CMD("qt_get_layout_types", QtGetLayoutTypes);
    CQTCL_BASE_ADD_CMD("qt_create_layout"   , QtCreateLayout);
    CQTCL_BASE_ADD_CMD("qt_add_child_widget", QtAddChildWidget);
    CQTCL_BASE_ADD_CMD("qt_add_stretch"     , QtAddStretch);
    CQTCL_BASE_ADD_CMD("qt_connect_widget"  , QtConnectWidget);
    CQTCL_BASE_ADD_CMD("qt_activate_slot"   , QtActivateSlot);
    CQTCL_BASE_ADD_CMD("qt_get_widgets"     , QtGetWidgets);

    CQTCL_BASE_ADD_CMD("qt_get_widget_data", QtGetWidgetData);
    CQTCL_BASE_ADD_CMD("qt_set_widget_data", QtSetWidgetData);

    CQTCL_BASE_ADD_CMD("qt_get_property", QtGetProperty);
    CQTCL_BASE_ADD_CMD("qt_set_property", QtSetProperty);
    CQTCL_BASE_ADD_CMD("qt_has_property", QtHasProperty);

    CQTCL_BASE_ADD_CMD("qt_sync", QtSync);

    CQTCL_BASE_ADD_CMD("qt_timer", Timer);

    CQTCL_BASE_ADD_CMD("perf", Perf);

    CQTCL_BASE_ADD_CMD("assert", Assert);

    CQTCL_BASE_ADD_CMD("sh", Shell);

    //---

    qtcl()->createAlias("echo", "puts");

    //---

    cmdsAdded = true;
  }
}

void
CQTclCmdBase::
addCommand(const QString &name, CQTclCmdProc *proc)
{
  proc->setName(name);

  auto *tclCmd = new CQTclCmd(this, name);

  proc->setTclCmd(tclCmd);

  commandNames_.push_back(name);

  commandProcs_[name] = proc;
}

bool
CQTclCmdBase::
processCmd(const QString &cmd, const Vars &vars)
{
  auto p = commandProcs_.find(cmd);

  if (p != commandProcs_.end()) {
    auto *proc = (*p).second;

    CQTclCmdArgs argv(cmd, vars);

    return proc->exec(argv);
  }

  //---

  if (cmd == "exit") { exit(0); }

  //---

  return false;
}

//------

// qt_get_widget_types
bool
CQTclCmdBase::
execQtGetWidgetTypesCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetWidgetTypesCmd");

  argv.addCmdArg("-set"   , CQTclCmdArg::Type::String , "set name");
  argv.addCmdArg("-simple", CQTclCmdArg::Type::Boolean, "only return simple types");

  if (! argv.parse())
    return false;

  auto setName = argv.getParseStr("set");
  auto simple  = argv.getParseBool("simple");

  QStringList names;

  if      (setName == ""  )
    names = CQWidgetFactoryMgrInst->widgetFactoryNames(simple);
  else if (setName == "qt")
    names = CQWidgetFactoryMgrInst->setWidgetFactoryNames("", simple);
  else
    names = CQWidgetFactoryMgrInst->setWidgetFactoryNames(setName, simple);

  return setCmdRc(names);
}

//------

// qt_create_widget
bool
CQTclCmdBase::
execQtCreateWidgetCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtCreateWidgetCmd");

  argv.addCmdArg("-parent", CQTclCmdArg::Type::String, "parent name");
  argv.addCmdArg("-type"  , CQTclCmdArg::Type::String, "widget type");
  argv.addCmdArg("-name"  , CQTclCmdArg::Type::String, "widget name");
  argv.addCmdArg("-set"   , CQTclCmdArg::Type::String, "widget set");

  if (! argv.parse())
    return false;

  auto typeName = argv.getParseStr("type");
  auto setName  = argv.getParseStr("set");

  if (typeName == "?") {
    QStringList names;

    if      (setName == ""  )
      names = CQWidgetFactoryMgrInst->widgetFactoryNames();
    else if (setName == "qt")
      names = CQWidgetFactoryMgrInst->setWidgetFactoryNames("");
    else
      names = CQWidgetFactoryMgrInst->setWidgetFactoryNames(setName);

    return setCmdRc(names);
  }

  QWidget *parentWidget = nullptr;

//QLayout *l = nullptr;

  if (argv.hasParseArg("parent")) {
    auto parentName = argv.getParseStr("parent");

    parentWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(parentName));

    if (! parentWidget)
      return errorMsg(QString("No parent '%1'").arg(parentName));

//  l = parentWidget->layout();
  }

  if (setName != "" && setName != "qt")
    typeName = setName + ":" + typeName;

  if (! CQWidgetFactoryMgrInst->isWidgetFactory(typeName))
    return errorMsg(QString("Invalid type '%1'").arg(typeName));

  auto *w = CQWidgetFactoryMgrInst->createWidget(typeName, parentWidget);

  if (! w)
    return errorMsg(QString("Failed to create '%1'").arg(typeName));

  auto name = argv.getParseStr("name");

  if (name != "")
    w->setObjectName(name);

//if (l)
//  l->addWidget(w);

//w->show();

  return setCmdRc(CQUtil::fullName(w));
}

//------

// qt_get_layout_types
bool
CQTclCmdBase::
execQtGetLayoutTypesCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetLayoutTypesCmd");

  if (! argv.parse())
    return false;

  auto names = CQWidgetFactoryMgrInst->layoutFactoryNames();

  return setCmdRc(names);
}

//------

// qt_create_layout
bool
CQTclCmdBase::
execQtCreateLayoutCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtCreateLayoutCmd");

  argv.addCmdArg("-parent", CQTclCmdArg::Type::String, "parent name");
  argv.addCmdArg("-type"  , CQTclCmdArg::Type::String, "layout type");
  argv.addCmdArg("-name"  , CQTclCmdArg::Type::String, "layout name");

  if (! argv.parse())
    return false;

  auto typeName = argv.getParseStr("type");

  if (typeName == "?") {
    auto names = CQWidgetFactoryMgrInst->layoutFactoryNames();

    return setCmdRc(names);
  }

  QWidget *parentWidget = nullptr;

  if (argv.hasParseArg("parent")) {
    auto parentName = argv.getParseStr("parent");

    parentWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(parentName));

    if (! parentWidget)
      return errorMsg(QString("No parent '%1'").arg(parentName));
  }
  else {
    return errorMsg("No parent");
  }

  if (! CQWidgetFactoryMgrInst->isLayoutFactory(typeName))
    return errorMsg(QString("Invalid type '%1'").arg(typeName));

  if (parentWidget->layout())
    return errorMsg(QString("Widget '%1' already has a layout").
             arg(CQUtil::fullName(parentWidget)));

  auto *l = CQWidgetFactoryMgrInst->createLayout(typeName, parentWidget);

  if (! l)
    return errorMsg(QString("Failed to create '%1'").arg(typeName));

  auto name = argv.getParseStr("name");

  if (name != "")
    l->setObjectName(name);

  return setCmdRc(CQUtil::fullName(l));
}

//------

// qt_add_child_widget
bool
CQTclCmdBase::
execQtAddChildWidgetCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtAddChildWidgetCmd");

  argv.addCmdArg("-parent" , CQTclCmdArg::Type::String , "parent name");
  argv.addCmdArg("-control", CQTclCmdArg::Type::String , "control name");
  argv.addCmdArg("-child"  , CQTclCmdArg::Type::String , "layout type");
  argv.addCmdArg("-stretch", CQTclCmdArg::Type::Integer, "stretch");
  argv.addCmdArg("-label"  , CQTclCmdArg::Type::String , "label");
  argv.addCmdArg("-row"    , CQTclCmdArg::Type::Integer, "grid row");
  argv.addCmdArg("-column" , CQTclCmdArg::Type::Integer, "grid column");

  if (! argv.parse())
    return false;

  //---

  auto parentName  = argv.getParseStr("parent");
  auto controlName = argv.getParseStr("control");

  QWidget *parentWidget = nullptr;

  if      (parentName != "") {
    parentWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(parentName));

    if (! parentWidget)
      return errorMsg(QString("No parent '%1'").arg(parentName));
  }
  else if (controlName != "") {
    auto *control = CQTclControlMgrInst->getControl(controlName);

    if (! control)
      return errorMsg(QString("No control '%1'").arg(controlName));

    parentWidget = control->getWidget();
  }
  else
    return errorMsg(QString("Missing -parent or -control"));

  //---

  if      (argv.hasParseArg("child")) {
    auto childName = argv.getParseStr("child");

    auto *childWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(childName));

    if (! childWidget)
      return errorMsg(QString("No widget '%1'").arg(childName));

    auto *qtab = qobject_cast<QTabWidget *>(parentWidget);

    if (qtab) {
      auto label = argv.getParseStr("label");

      qtab->addTab(childWidget, label);
    }
    else {
      auto *layout = parentWidget->layout();

      if (! layout)
        layout = new QVBoxLayout(parentWidget);

      auto *gridLayout = qobject_cast<QGridLayout *>(layout);
      auto *formLayout = qobject_cast<QFormLayout *>(layout);

      if      (gridLayout) {
        int row = argv.getParseInt("row");
        int col = argv.getParseInt("column");

        gridLayout->addWidget(childWidget, row, col);
      }
      else if (formLayout) {
        auto label = argv.getParseStr("label");

        if (label != "")
          formLayout->addRow(label, childWidget);
        else
          formLayout->addRow(childWidget);
      }
      else
        layout->addWidget(childWidget);
    }

    return setCmdRc(CQUtil::fullName(childWidget));
  }
  else if (argv.hasParseArg("stretch")) {
    int stretch = argv.getParseInt("stretch");

    auto *layout = qobject_cast<QBoxLayout *>(parentWidget->layout());

    if (! layout)
      return errorMsg(QString("No layout for '%1'").arg(parentName));

    auto *gridLayout = qobject_cast<QGridLayout *>(layout);

    if (gridLayout) {
      int row = -1;
      int col = -1;

      if (argv.hasParseArg("row"))
        row = argv.getParseInt("row");

      if (argv.hasParseArg("column"))
        col = argv.getParseInt("column");

      if      (row >= 0)
        gridLayout->setRowStretch(row, stretch);
      else if (col >= 0)
        gridLayout->setColumnStretch(row, stretch);
    }
    else
      layout->addStretch(stretch);

    return setCmdRc(QString());
  }
  else {
    return errorMsg("Specify -child or -layout");
  }
}

//------

// qt_add_stretch
bool
CQTclCmdBase::
execQtAddStretchCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtAddStretchCmd");

  argv.addCmdArg("-parent", CQTclCmdArg::Type::String, "parent name");

  if (! argv.parse())
    return false;

  auto parentName = argv.getParseStr("parent");

  auto *parentWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(parentName));

  if (! parentWidget)
    return errorMsg(QString("No parent '%1'").arg(parentName));

  auto *layout = parentWidget->layout();

  if (! layout)
    layout = new QVBoxLayout(parentWidget);

  auto *boxLayout = qobject_cast<QBoxLayout *>(layout);

  if (boxLayout)
    boxLayout->addStretch(1);

  return true;
}

//------

// qt_connect_widget
bool
CQTclCmdBase::
execQtConnectWidgetCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtConnectWidgetCmd");

  argv.addCmdArg("-name"  , CQTclCmdArg::Type::String, "widget name");
  argv.addCmdArg("-signal", CQTclCmdArg::Type::String, "signal name");
  argv.addCmdArg("-proc"  , CQTclCmdArg::Type::String, "tcl proc name");

  if (! argv.parse())
    return false;

  auto name = argv.getParseStr("name");

  auto *widget = qobject_cast<QWidget *>(CQUtil::nameToObject(name));

  if (! widget)
    return errorMsg(QString("No widget '%1'").arg(name));

  //---

  auto signalName = argv.getParseStr("signal");
  auto normSignal = QString(QMetaObject::normalizedSignature(signalName.toLatin1().constData()));

  // replace routine name in signal name
  auto ind = normSignal.indexOf("(");
  if (ind < 0)
    return errorMsg(QString("Invalid signal name '%1'").arg(signalName));

  auto slotName = QString("1") + "connectSlot" + normSignal.mid(ind);

  //---

  auto procName = argv.getParseStr("proc");

  auto *slot = new CQTclCmdBaseSlot(this, widget, procName);

  //---

  auto *list  = qobject_cast<QListView *>(widget);
  auto *table = qobject_cast<QTableView *>(widget);

  if      (list) {
    if (normSignal == "valueChanged()") {
      auto *sm = list->selectionModel();
      if (! sm) return false;

      QObject::connect(sm, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                       slot, slotName.toLatin1().constData());

      return true;
    }
  }
  else if (table) {
    if (normSignal == "valueChanged()") {
      auto *sm = table->selectionModel();
      if (! sm) return false;

      QObject::connect(sm, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                       slot, slotName.toLatin1().constData());

      return true;
    }
  }

  auto signalName2 = QString("2") + normSignal;

  QObject::connect(widget, signalName2.toLatin1().constData(),
                   slot, slotName.toLatin1().constData());

  return true;
}

//------

// qt_activate_slot
bool
CQTclCmdBase::
execQtActivateSlotCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtActivateSlotCmd");

  argv.addCmdArg("-name", CQTclCmdArg::Type::String, "widget name");
  argv.addCmdArg("-slot", CQTclCmdArg::Type::String, "slot name");
  argv.addCmdArg("-args", CQTclCmdArg::Type::String, "slot args");

  if (! argv.parse())
    return false;

  auto name = argv.getParseStr("name");

  auto *object = CQUtil::nameToObject(name);

  if (! object)
    return errorMsg(QString("No object '%1'").arg(name));

  auto slotName = argv.getParseStr("slot").toStdString();
  auto argsStr  = argv.getParseStr("args").toStdString();

  if (! CQUtil::activateSlot(object, slotName.c_str(), argsStr.c_str()))
    return errorMsg(QString("Invalid slot '%1'").arg(slotName.c_str()));

  return true;
}

//------

// qt_get_widgets
bool
CQTclCmdBase::
execQtGetWidgetsCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetWidgetsCmd");

  argv.addCmdArg("-parent" , CQTclCmdArg::Type::String , "parent name");
  argv.addCmdArg("-control", CQTclCmdArg::Type::String , "control name");
  argv.addCmdArg("-name"   , CQTclCmdArg::Type::String, "name pattern");

  if (! argv.parse())
    return false;

  //---

  auto parentName  = argv.getParseStr("parent");
  auto controlName = argv.getParseStr("control");

  QWidget *parentWidget = nullptr;

  if      (parentName != "") {
    parentWidget = qobject_cast<QWidget *>(CQUtil::nameToObject(parentName));

    if (! parentWidget)
      return errorMsg(QString("No parent '%1'").arg(parentName));
  }
  else if (controlName != "") {
    auto *control = CQTclControlMgrInst->getControl(controlName);

    if (! control)
      return errorMsg(QString("No control '%1'").arg(controlName));

    parentWidget = control->getWidget();
  }

  //---

  auto name = argv.getParseStr("name");

  auto objects = CQUtil::nameToObjects(parentWidget, name);

  QStringList names;

  for (auto *object : objects) {
    auto path = CQUtil::fullName(object);

    names.push_back(path);
  }

  return setCmdRc(names);
}

//------

// qt_get_widget_data
bool
CQTclCmdBase::
execQtGetWidgetDataCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetWidgetDatCmd");

  argv.addCmdArg("-widget", CQTclCmdArg::Type::String, "widget name");
  argv.addCmdArg("-name"  , CQTclCmdArg::Type::String, "data name");

  if (! argv.parse())
    return false;

  auto widgetName = argv.getParseStr("widget");

  auto *w = qobject_cast<QWidget *>(CQUtil::nameToObject(widgetName));

  if (! w)
    return errorMsg(QString("No widget '%1'").arg(widgetName));

  auto name = argv.getParseStr("name");

  //---

  QVariant value;

  if (name == "value") {
    if (! CQUtil::getWidgetValue(w, value))
      return false;
  }
  else
    return errorMsg(QString("Invalid data name '%1'").arg(name));

  return setCmdRc(value);
}

//------

// qt_set_widget_data
bool
CQTclCmdBase::
execQtSetWidgetDataCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetWidgetDatCmd");

  argv.addCmdArg("-widget", CQTclCmdArg::Type::String, "widget name");
  argv.addCmdArg("-name"  , CQTclCmdArg::Type::String, "data name");
  argv.addCmdArg("-value" , CQTclCmdArg::Type::String, "data value");

  if (! argv.parse())
    return false;

  auto widgetName = argv.getParseStr("widget");

  auto *w = qobject_cast<QWidget *>(CQUtil::nameToObject(widgetName));

  if (! w)
    return errorMsg(QString("No widget '%1'").arg(widgetName));

  auto name = argv.getParseStr("name");

  //---

  auto value = argv.getParseStr("value");

  if (name == "value") {
    s_slots_enabled = false;

    auto rc = CQUtil::setWidgetValue(w, value);

    s_slots_enabled = true;

    if (! rc)
      return false;
  }
  else
    return errorMsg(QString("Invalid data name '%1'").arg(name));

  return true;
}

//------

// qt_get_property
bool
CQTclCmdBase::
execQtGetPropertyCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtGetPropertyCmd");

  argv.addCmdArg("-object"  , CQTclCmdArg::Type::String, "object name");
  argv.addCmdArg("-property", CQTclCmdArg::Type::String, "property name");

  if (! argv.parse())
    return false;

  auto objectName = argv.getParseStr("object");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj)
    return errorMsg(QString("No object '%1'").arg(objectName));

  auto propName = argv.getParseStr("property");

  //---

  QVariant v;

  if (! CQUtil::getProperty(obj, propName, v))
    return errorMsg(QString("Failed to get property '%1' for '%2'").arg(propName).arg(objectName));

  return setCmdRc(v);
}

//------

// qt_set_property
bool
CQTclCmdBase::
execQtSetPropertyCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtSetPropertyCmd");

  argv.addCmdArg("-object"  , CQTclCmdArg::Type::String, "object name");
  argv.addCmdArg("-property", CQTclCmdArg::Type::String, "property name");
  argv.addCmdArg("-value"   , CQTclCmdArg::Type::String, "property value");

  if (! argv.parse())
    return false;

  auto objectName = argv.getParseStr("object");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj)
    return errorMsg(QString("No object '%1'").arg(objectName));

  auto propName = argv.getParseStr("property");
  auto value    = argv.getParseStr("value");

  if      (propName == "items") {
    auto *combo = qobject_cast<QComboBox *>(obj);
    auto *list  = qobject_cast<QListView *>(obj);
    auto *table = qobject_cast<QTableView *>(obj);

    QStringList strs;

    CQTclUtil::splitList(value, strs);

    if      (combo) {
      combo->clear();

      combo->addItems(strs);

      return true;
    }
    else if (list) {
      auto *model = list->model();
      assert(model);

      int nr = strs.length();

      while (model->rowCount() > nr) model->removeRow(0);
      while (model->rowCount() < nr) model->insertRow(0);

      for (int r = 0; r < nr; ++r) {
        auto ind = model->index(r, 0, QModelIndex());

        model->setData(ind, strs[r], Qt::DisplayRole);
      }

      return true;
    }
    else if (table) {
      auto *model = table->model();
      assert(model);

      int nr = strs.length();

      while (model->rowCount() > nr) model->removeRow(0);
      while (model->rowCount() < nr) model->insertRow(0);

      int nc = 0;

      for (int r = 0; r < nr; ++r) {
        QStringList strs1;

        CQTclUtil::splitList(strs[r], strs1);

        int nc1 = strs1.length();

        nc = std::max(nc, nc1);
      }

      while (model->columnCount() > nc) model->removeColumn(0);
      while (model->columnCount() < nc) model->insertColumn(0);

      for (int r = 0; r < nr; ++r) {
        QStringList strs1;

        CQTclUtil::splitList(strs[r], strs1);

        int nc1 = strs1.length();

        for (int c = 0; c < nc1; ++c) {
          auto ind = model->index(r, c, QModelIndex());

          model->setData(ind, strs1[c], Qt::DisplayRole);
        }
      }

      return true;
    }
  }
  else if (propName == "value") {
    auto *list  = qobject_cast<QListView *>(obj);
    auto *table = qobject_cast<QTableView *>(obj);

    if      (list) {
      auto *model = list->model();
      assert(model);

      auto *sm = list->selectionModel();
      if (! sm) return false;

      bool ok;
      int r = value.toInt(&ok);
      if (! ok) return false;

      QItemSelection sel;

      auto ind = model->index(r, 0, QModelIndex());

      sel.select(ind, ind);

      sm->select(sel, QItemSelectionModel::Select | QItemSelectionModel::Clear);

      return true;
    }
    else if (table) {
      auto *model = table->model();
      assert(model);

      auto *sm = table->selectionModel();
      if (! sm) return false;

      bool ok;
      int r = value.toInt(&ok);
      if (! ok) return false;

      QItemSelection sel;

      auto ind = model->index(r, 0, QModelIndex());

      sel.select(ind, ind);

      sm->select(sel, QItemSelectionModel::Select | QItemSelectionModel::Clear);

      return true;
    }
  }

  if (! CQUtil::setProperty(obj, propName, value))
    return errorMsg(QString("Failed to set property '%1' for '%2'").
             arg(propName).arg(objectName));

  return true;
}

//------

// qt_has_property
bool
CQTclCmdBase::
execQtHasPropertyCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtHasPropertyCmd");

  argv.addCmdArg("-object"  , CQTclCmdArg::Type::String , "object name");
  argv.addCmdArg("-property", CQTclCmdArg::Type::String , "property name");
  argv.addCmdArg("-writable", CQTclCmdArg::Type::Boolean, "property is writable");

  if (! argv.parse())
    return false;

  auto objectName = argv.getParseStr("object");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj)
    return errorMsg(QString("No object '%1'").arg(objectName));

  auto propName = argv.getParseStr("property");

  bool b;

  if (argv.hasParseArg("writable"))
    b = CQUtil::hasWritableProperty(obj, propName);
  else
    b = CQUtil::hasProperty(obj, propName);

  return setCmdRc(b);
}

//------

// qt_sync
bool
CQTclCmdBase::
execQtSyncCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execQtSyncCmd");

  argv.addCmdArg("-n", CQTclCmdArg::Type::Integer, "loop count");

  if (! argv.parse())
    return false;

  int n = 1;

  if (argv.hasParseArg("n"))
    n = argv.getParseInt("n");

  for (int i = 0; i < n; ++i) {
    //qApp->flush();

    qApp->processEvents();
  }

  return true;
}

//------

// qt_timer
bool
CQTclCmdBase::
execTimerCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execTimercCmd");

  argv.addCmdArg("-delay", CQTclCmdArg::Type::Integer, "delay in ms");
  argv.addCmdArg("-proc" , CQTclCmdArg::Type::String , "proc to call");

  if (! argv.parse())
    return false;

  int delay = 100;

  if (argv.hasParseArg("delay"))
    delay = argv.getParseInt("delay");

  QString procName;

  if (argv.hasParseArg("proc"))
    procName = argv.getParseStr("proc");

  if (procName == "")
    return errorMsg("No proc");

  auto *slot = new CQTclCmdBaseSlot(this, nullptr, procName);

  QTimer::singleShot(delay, slot, SLOT(timerSlot()));

  return true;
}

//------

// perf
bool
CQTclCmdBase::
execPerfCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execPerfCmd");

  argv.addCmdArg("-start_recording", CQTclCmdArg::Type::Boolean, "start recording");
  argv.addCmdArg("-end_recording"  , CQTclCmdArg::Type::Boolean, "end recording"  );
  argv.addCmdArg("-tracing"        , CQTclCmdArg::Type::SBool  , "enable tracing" );
  argv.addCmdArg("-debug"          , CQTclCmdArg::Type::SBool  , "enable debug"   );

  if (! argv.parse())
    return false;

  //---

  if (argv.hasParseArg("start_recording"))
    CQPerfMonitorInst->startRecording();

  if (argv.hasParseArg("end_recording"))
    CQPerfMonitorInst->stopRecording();

  if (argv.hasParseArg("tracing"))
    CQPerfMonitorInst->setEnabled(argv.getParseBool("tracing"));

  if (argv.hasParseArg("debug"))
    CQPerfMonitorInst->setDebug(argv.getParseBool("debug"));

  return true;
}

//------

// assert
bool
CQTclCmdBase::
execAssertCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execAssertCmd");

  if (! argv.parse())
    return false;

  //---

  const Vars &pargs = argv.getParseArgs();

  auto expr = (! pargs.empty() ? pargs[0].toString() : "");

  auto expr1 = QString("expr {%1}").arg(expr);

  return qtcl()->eval(expr1, /*showError*/true, /*showResult*/false);
}

//------

// sh
bool
CQTclCmdBase::
execShellCmd(CQTclCmdArgs &argv)
{
  CQPerfTrace trace("CQTclCmdBase::execShellCmd");

  if (! argv.parse())
    return false;

  //---

  const Vars &pargs = argv.getParseArgs();

  auto cmd = (! pargs.empty() ? pargs[0].toString() : "");

  //---

  if (cmd == "")
    return errorMsg("No command");

  int rc = system(cmd.toLatin1().constData());

  return setCmdRc(rc);
}

//------

// help
bool
CQTclCmdBase::
execHelpCmd(CQTclCmdArgs &)
{
  for (auto &name : commandNames_)
    app_->outputText(name + "\n");

  return true;
}

//------

QStringList
CQTclCmdBase::
stringToCmdData(const QString &str) const
{
  return stringToLines(str);
}

//------

bool
CQTclCmdBase::
setCmdRc(int rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdRc(double rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdRc(const QString &rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdRc(const QVariant &rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdRc(const QStringList &rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdRc(const QVariantList &rc)
{
  qtcl()->setResult(rc);

  qtcl()->outputResult();

  return true;
}

bool
CQTclCmdBase::
setCmdError(const QString &msg)
{
  (void) errorMsg(msg);

  (void) setCmdRc(QString());

  return false;
}

//------

QStringList
CQTclCmdBase::
stringToCmds(const QString &str) const
{
  return stringToLines(str);
}

//------

QStringList
CQTclCmdBase::
stringToLines(const QString &str) const
{
  auto lines = str.split('\n', Qt::SkipEmptyParts);

  QStringList lines1;

  int i = 0;

  for ( ; i < lines.size(); ++i) {
    auto line = lines[i];

    bool join;

    while (! isCompleteLine(line, join)) {
      ++i;

      if (i >= lines.size())
        break;

      const auto &line1 = lines[i];

      if (! join)
        line += "\n" + line1;
      else
        line += line1;
    }

    lines1.push_back(line);
  }

  return lines1;
}

//------

bool
CQTclCmdBase::
isCompleteLine(QString &str, bool &join)
{
  join = false;

  if (! str.length())
    return true;

  if (str[str.size() - 1] == '\\') {
    str = str.mid(0, str.length() - 1);
    join = true;
    return false;
  }

  //---

  CQStrParse line(str);

  line.skipSpace();

  while (! line.eof()) {
    if      (line.isChar('{')) {
      if (! line.skipBracedString())
        return false;
    }
    else if (line.isChar('\"') || line.isChar('\'')) {
      if (! line.skipString())
        return false;
    }
    else {
      line.skipNonSpace();
    }

    line.skipSpace();
  }

  return true;
}

void
CQTclCmdBase::
parseLine(const QString &line, bool log)
{
  if (! qtcl()->eval(line, /*showError*/true, /*showResult*/log))
    (void) errorMsg("Invalid line: '" + line + "'");
}

bool
CQTclCmdBase::
errorMsg(const QString &msg)
{
  app_->errorMsg(msg);

  bool b;

  if (CEnvInst.get("CQ_TCL_CMD_PEDANTIC", b) && b)
    assert(false);

  return false;
}

//---

CQTclCmdBaseSlot::
CQTclCmdBaseSlot(CQTclCmdBase *base, QWidget *w, const QString &procName) :
 base_(base), w_(w), procName_(procName) {
}

void
CQTclCmdBaseSlot::
connectSlot()
{
  execProc();
}

void
CQTclCmdBaseSlot::
connectSlot(bool b)
{
  auto args = QString(b ? "1" : "0");

  execProc(args);
}

void
CQTclCmdBaseSlot::
connectSlot(int i)
{
  QString args;

  args.setNum(i);

  execProc(args);
}

void
CQTclCmdBaseSlot::
connectSlot(double r)
{
  QString args;

  args.setNum(r);

  execProc(args);
}

void
CQTclCmdBaseSlot::
connectSlot(const QString &s)
{
  auto args = QString("{%1}").arg(s);

  execProc(args);
}

void
CQTclCmdBaseSlot::
execProc(const QString &args)
{
  if (! s_slots_enabled)
    return;

  auto cmd = procName_;

  QObject *obj;

  if (w_)
    obj = w_.data();

  if (! obj)
    obj = sender();

  if (obj)
     cmd += QString(" {%1}").arg(CQUtil::fullName(obj));

  if (args != "")
    cmd += " " + args;

  (void) base_->qtcl()->eval(cmd, /*showError*/true, /*showResult*/false);
}
