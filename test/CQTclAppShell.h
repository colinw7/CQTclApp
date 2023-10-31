#ifndef CQTclAppShell_H
#define CQTclAppShell_H

#include <QFrame>

class CQTclApp;

namespace CQCommand {
class ScrollArea;
}

class CQTclAppShell : public QFrame {
  Q_OBJECT

 public:
  CQTclAppShell(CQTclApp *app);
 ~CQTclAppShell();

  QFrame *control() const { return control_; }

  QSize sizeHint() const override;

 public Q_SLOTS:
  void executeCommand(const QString &);

 private:
  CQTclApp*              app_     { nullptr };
  QFrame*                control_ { nullptr };
  CQCommand::ScrollArea* command_ { nullptr };
};

#endif
