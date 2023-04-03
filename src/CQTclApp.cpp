#include <CQTclApp.h>
#include <CQTclCmdBase.h>
#include <CQTclReadLine.h>
#include <CQTclUtil.h>
#include <CQMsgHandler.h>
#include <CQStrParse.h>
#include <CFile.h>

#ifdef CQ_APP_H
#include <CQApp.h>
#else
#include <QApplication>
#endif

#include <iostream>

//------

CQTclApp::
CQTclApp(int &argc, char **argv)
{
  CQMsgHandler::install();

#ifdef CQ_APP_H
  app_ = new CQApp(argc, argv);
#else
  app_ = new QApplication(argc, argv);
#endif

//qtcl_ = new CQTcl;

  cmdBase_ = new CQTclCmdBase(this);

  addCommands();
}

CQTclApp::
~CQTclApp()
{
  delete readLine_;
  delete app_;
}

void
CQTclApp::
addCommands()
{
  cmdBase_->addCommands();
}

void
CQTclApp::
exec()
{
  bool isLoop = true;

  if (! isLoop)
    app_->exec();
  else
    loop();
}

void
CQTclApp::
loadFile(const QString &fileName)
{
  CFile file(fileName.toStdString());

  std::vector<std::string> lines;

  file.toLines(lines);

  std::string line1;

  for (const auto &line : lines) {
    if (line1 != "")
      line1 += "\n";

    line1 += line;

    if (CTclUtil::isCompleteLine(line1)) {
      parseLine(line1.c_str());

      line1 = "";
    }
  }

  if (line1 != "")
    parseLine(line1.c_str());
}

void
CQTclApp::
loop()
{
  if (! readLine_) {
    int rlTimeout = 10;

    readLine_ = new CQTclReadLine(this);

    readLine_->enableTimeoutHook(rlTimeout);
  }

  readLine_->loop();
}

void
CQTclApp::
timeout()
{
  if (! qApp->activeModalWidget())
    qApp->processEvents();
}

void
CQTclApp::
errorMsg(const QString &msg)
{
  std::cerr << msg.toStdString() << "\n";
}

bool
CQTclApp::
isCompleteLine(QString &str, bool &join)
{
  return cmdBase_->isCompleteLine(str, join);
}

void
CQTclApp::
parseLine(const QString &line, bool log)
{
  cmdBase_->parseLine(line, log);
}

void
CQTclApp::
outputText(const QString &text)
{
  std::cout << text.toStdString();
}
