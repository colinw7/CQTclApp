#ifndef CQTclAppShell_H
#define CQTclAppShell_H

#include <QDialog>

class CQTclApp;

namespace CQCommand {
class ScrollArea;
}

class CQTclAppShell : public QDialog {
  Q_OBJECT

 public:
  CQTclAppShell(CQTclApp *app);
 ~CQTclAppShell();

  QSize sizeHint() const override;

 public Q_SLOTS:
  void executeCommand(const QString &);

 private:
  CQTclApp*              app_     { nullptr };
  CQCommand::ScrollArea* command_ { nullptr };
};

#endif
