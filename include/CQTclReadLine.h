#ifndef CQTclReadLine_H
#define CQTclReadLine_H

class CQTclApp;

#include <CReadLine.h>

class CQTclReadLine : public CReadLine {
 public:
  CQTclReadLine(CQTclApp *app);

  void timeout() override;

  void loop();

 private:
  CQTclApp *app_ { nullptr };
};

#endif
