#ifndef CQTclApp_H
#define CQTclApp_H

#include <QObject>

class CQTclReadLine;
class CQTclCmdBase;
class CQTcl;

class QApplication;

class CQTclApp : QObject {
  Q_OBJECT

 public:
  CQTclApp(int &argc, char **argv);
 ~CQTclApp();

  void addCommands();

  void exec();

  void loadFile(const QString &fileName);

  void loop();

  void timeout();

  void errorMsg(const QString &msg);

  bool isCompleteLine(QString &str, bool &join);

  void parseLine(const QString &line, bool log=false);

  void outputText(const QString &text);

 private:
  QApplication*  app_      { nullptr };
//CQTcl*         qtcl_     { nullptr };
  CQTclCmdBase*  cmdBase_  { nullptr };
  CQTclReadLine* readLine_ { nullptr };
};

#endif
