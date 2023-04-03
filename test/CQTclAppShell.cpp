#include <CQTclAppShell.h>
#include <CQTclApp.h>

#include <CQCommand.h>
#include <CQUtil.h>
#include <COSExec.h>

#include <QVBoxLayout>

#include <iostream>

class TestCmdWidget : public CQCommand::CommandWidget {
 public:
  TestCmdWidget(CQCommand::ScrollArea *scrollArea) :
   CQCommand::CommandWidget(scrollArea) {
  }

  bool complete(const QString &text, int pos,
                QString &newText, CompleteMode completeMode) const override {
    return CQCommand::CommandWidget::complete(text, pos, newText, completeMode);
  }

  bool isCompleteLine(const QString &str) const override {
    return CQCommand::CommandWidget::isCompleteLine(str);
  }
};

//---

class TestCmdScroll : public CQCommand::ScrollArea {
 public:
  TestCmdScroll(QWidget *parent=nullptr) :
   CQCommand::ScrollArea(parent) {
  }

  CQCommand::CommandWidget *createCommandWidget() const override {
    auto *scrollArea = dynamic_cast<const CQCommand::ScrollArea *>(this);

    return new TestCmdWidget(const_cast<CQCommand::ScrollArea *>(scrollArea));
  }
};

//---

CQTclAppShell::
CQTclAppShell(CQTclApp *app) :
 app_(app)
{
  auto *layout = CQUtil::makeLayout<QVBoxLayout>(this, 0, 0);

  command_ = new TestCmdScroll(this);

  command_->getCommand()->setPrompt("> ");

  connect(command_, SIGNAL(executeCommand(const QString &)),
          this, SLOT(executeCommand(const QString &)));

  layout->addWidget(command_);
}

CQTclAppShell::
~CQTclAppShell()
{
}

void
CQTclAppShell::
executeCommand(const QString &str)
{
  std::cerr << str.toStdString() << "\n";

  COSExec::grabOutput(true, true);

  app_->parseLine(str);

  std::string ostr;

  COSExec::readGrabbedOutput(ostr);

  COSExec::ungrabOutput();

  command_->outputText(ostr.c_str());
}

QSize
CQTclAppShell::
sizeHint() const
{
  return QSize(1200, 1600);
}
