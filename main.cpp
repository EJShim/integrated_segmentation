#include <QApplication>
#include <QFile>
#include "E_Window.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QFile File(":styles/black.css");
  File.open(QFile::ReadOnly);
  QString StyleSheet = QLatin1String(File.readAll());
  app.setStyleSheet(StyleSheet);

  E_Window mainwindow;
  mainwindow.show();

  return app.exec();
}