#include <CQTclApp.h>
#include <CQTclAppShell.h>
#include <iostream>

int
main(int argc, char **argv)
{
  bool                     shell = false;
  std::vector<std::string> files;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      auto arg = std::string(&argv[i][1]);

      if (arg == "shell")
        shell = true;
      else
        std::cerr << "Invalid arg '" << argv[i] << "'\n";
    }
    else {
      files.emplace_back(argv[i]);
    }
  }

  auto *app = new CQTclApp(argc, argv);

  CQTclAppShell *dialog = nullptr;

  if (shell) {
    dialog = new CQTclAppShell(app);

    dialog->show();
  }

  for (const auto &file : files)
    app->loadFile(QString::fromStdString(file));

  app->exec();

  return 0;
}
