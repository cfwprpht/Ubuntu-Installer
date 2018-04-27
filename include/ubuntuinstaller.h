#ifndef UBUNTUINSTALLER_H
#define UBUNTUINSTALLER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QPixmap>
#include <QImage>
#include <QProcess>
#include <QEventLoop>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui { class UbuntuInstaller; }

class UbuntuInstaller : public QMainWindow {
    Q_OBJECT

public:
    explicit UbuntuInstaller(QWidget *parent = 0);
    ~UbuntuInstaller();

private slots:
    void on_actionThis_thing_triggered();
    void on_buttonOpen_clicked();
    void on_buttonInstall_clicked();
    void on_buttonAbort_clicked();
    void on_label_linkActivated(const QString &link);

private:
    Ui::UbuntuInstaller        *ui;
    static QImage              icon;
    static QGraphicsScene      *scene;
    static QGraphicsPixmapItem *item;
    bool MakeScript(QString filename, QString arg);
    bool MakeScript(QString filename, QStringList arglist);
    bool OnFlyScript(QString filename, QString arg);
    bool OnFlyScript(QString filename, QStringList arglist);
    char *ToChar(QString toConvert);
    static void Error(QString message);
    bool Run(QString command, QString arguments, QString *result);
    bool Run(QString command, QStringList arguments, QString *result);
    QString Run(QString command, QString arguments);
    QString Run(QString command, QStringList arguments);
    bool CopyDirectory(QString source, QString destination);
    bool MakeDesktopEntry(void);
    bool Install(QString src, QString dest);
    bool MakeUninstallScript(void);
    bool MakeUninstallDesktopEntry(void);
    bool MakeSudoSymLink(void);
    bool CheckInstallOk(void);
    bool CheckInstallClean(void);
    bool Clean(void);
    bool Unzip(void);
    bool MakeWorkingDir(void);
};

#endif // UBUNTUINSTALLER_H
