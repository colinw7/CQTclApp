#include <CQTclReadLine.h>
#include <CQTclApp.h>

CQTclReadLine::
CQTclReadLine(CQTclApp *app) :
 app_(app)
{
}

void
CQTclReadLine::
timeout()
{
  app_->timeout();
}

void
CQTclReadLine::
loop()
{
  for (;;) {
    setPrompt("> ");

    QString line = readLine().c_str();

    bool join;

    while (! app_->isCompleteLine(line, join)) {
      setPrompt("+> ");

      QString line1 = readLine().c_str();

      if (! join)
        line += "\n" + line1;
      else
        line += line1;
    }

    app_->parseLine(line);

    addHistory(line.toStdString());
  }
}
