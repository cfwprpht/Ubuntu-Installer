#include "../include/ubuntuinstaller.h"
#include "ui_ubuntuinstaller.h"
#include "../include/sudialog.h"

bool alldone        = false;
QWidget             *this_widget;
QString             home;
QString             work_dir;
QString             installTo;
QString             temp_name = "your_temp_folder";
QString             appname = "your_app_name";
QImage              UbuntuInstaller::icon;
QGraphicsScene      *UbuntuInstaller::scene;
QGraphicsPixmapItem *UbuntuInstaller::item;

// A List of files that need to be set to executable.
QStringList         chmodx;

// Desktop Entry.
QStringList         desktop = {
    "#!/usr/bin/env xdg-open\n",
    "[Desktop Entry]\n",
    "Version=1.0\n",
    "Name=YOUR_APP_NAME\n",
    "Comment=YOUR_APP_COMMENT.\n",
    "GenericName=YOUR_APP_GENERIC_NAME.\n",
    "Path=$INSP\n",
    "Exec=$INSPYOUR_APP_NAME\n",
    "Icon=$HOME/.local/share/icons/YOUR_APP_NAME.png\n",
    "Type=Application\n",
    "Terminal=true\n",
    "StartupNotify=false\n",
    "Categories=GTK;GNOME;Application;Development;\n",
    "MimeType=text/x-python;application/x-shellscript;\n",
    /*"Actions=Options;ShowPayloads;\n",*/
    "Keywords=YOUR_APP_KEYWORDS;\n\n",
    /*"[Desktop Action Options]\n",
    "Name=YOUR_APP_NAME\n",
    "Exec=$INSPYOUR_APP_NAME %U\n\n",
    "[Desktop Action ShowPayloads]\n",
    "Name=YOUR_APP_NAME\n",
    "Exec=nautilus --new-window /\n",*/
};

// Uninstall Desktop Entry.
QStringList         uidesktop = {
    "#!/usr/bin/env xdg-open\n",
    "[Desktop Entry]\n",
    "Version=1.0\n",
    "Name=YOUR_APP_NAME Uninstall\n",
    "Comment=Uninstall.\n",
    "GenericName=Uninstall YOUR_APP_NAME.\n",
    "Path=$INSP\n",
    "Exec=$INSPuninstall.sh\n",
    "Icon=$HOME/.local/share/icons/YOUR_APP_NAME.png\n",
    "Type=Application\n",
    "Terminal=true\n",
    "StartupNotify=false\n",
    "Categories=GTK;GNOME;Application;Development;\n",
    "MimeType=text/x-python;application/x-shellscript;\n",
    "Keywords=YOUR_APP_KEYWORD;",
};

// Remove Desktop entry from the launcher.
QStringList         unpinentry = {
    "# Remove Desktop Shortcut from the launcher.\n",
    "# Get List of all sticky desktop shortcuts.\n",
    "list=$(gsettings get com.canonical.Unity.Launcher favorites)\n\n",
    "# Now that we have them in a list, let's find our application entry and remove it.\n",
    "newlist=$(echo $list | sed \"s/'application:\"/\"/YOUR_APP_NAME.desktop', //\") \"\n",
    "gsettings set com.canonical.Unity.Launcher favorites \"$newlist\"\n",
};

UbuntuInstaller::UbuntuInstaller(QWidget *parent) : QMainWindow(parent), ui(new Ui::UbuntuInstaller) {    
    // Initializer.
    ui->setupUi(this);

    // Set a fixed size for this form.
    this->setFixedSize(471, 422);

    // Set home path as initial directory.
    ui->textBox->setText(home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());

    // Set temporary working folder.
    work_dir = SUDialog::work_dir = home + temp_name;

    // Identifyer to know if we should clean up a unsuccessful install.
    alldone = true;

    // Used for the MessageBox to point to the parent.
    this_widget = this;

    // Draw the app icon to the gui.
    QImage img(":/icons/Software.png");
    scene = new QGraphicsScene();
    item = new QGraphicsPixmapItem();
    icon = img.scaled(72, 72, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);    
    item->setPixmap(QPixmap::fromImage(icon));
    scene->addItem(item);
    ui->pictureBoxIcon->setScene(scene);
    ui->pictureBoxIcon->show();
}

UbuntuInstaller::~UbuntuInstaller() {
    // Have the installer found some problem ?
    if (!alldone) {
        QDir check(installTo);
        if (check.exists()) {
            if (!check.removeRecursively())
                Error("\nCouldn't completely remove the Installation Directory.\n\nPlease delete:\n" + installTo + "on your own.");
        }

        // Remove all other static files too.
        QFile *checkf = new QFile(home + "/.local/share/icons/" + appname + ".png");
        if (checkf->exists()) {
            checkf->remove();
            if (checkf->exists()) Error("\nCouldn't remove:\n\n" + home + "/.local/share/icons/" + appname + ".png\n\nplease remove it manually.");
        }

        delete checkf;
        checkf = new QFile(home + "/.local/share/applications/" + appname + ".desktop");
        if (checkf->exists()) {
            checkf->remove();
            if (checkf->exists()) Error("\nCouldn't remove:\n\n" + home + "/.local/share/applications/" + appname + ".desktop\n\nplease remove it manually.");
        }

        delete checkf;
        checkf = new QFile(home + "/.local/share/applications/" + appname + ".uninstall.desktop");
        if (checkf->exists()) {
            checkf->remove();
            if (checkf->exists()) Error("\nCouldn't remove:\n\n" + home + "/.local/share/applications/" + appname + ".uninstall.desktop\n\nplease remove it manually.");
        }

        delete checkf;
        /*checkf = new QFile("/etc/profile.d/supwd.sh");
        if (checkf->exists()) {
            if (!OnFlyScript("uninstall_su.sh", ("echo " + SUDialog::supassword + " | sudo -S rm -f \"/etc/profile.d/supwd.sh\"")))
                Error("\nCouldn't clean sudo bootup script.\n\nPlease remove it manually.");
        }
        delete checkf;*/

        if (ui->checkBoxLauncher->checkState() == Qt::Checked) {
            QString appdb, error;
            if (Run("gsettings", QStringList() << "get" << "com.canonical.Unity.Launcher" << "favorites", &appdb)) {
                if (appdb.contains(" 'application://" + appname + ".desktop")) {
                    if (Run("sh", QStringList() << "sed" << "s/'application:\"/\"/" + appname + ".desktop', //" << appdb, &appdb)) {
                        if (!Run("gsettings", QStringList() << "set" << "com.canonical.Unity.Launcher" << "favorites" << appdb, &error)) {
                            Error("\nCouldn't unstick application from the launcher.");
                        }
                    } else Error("\nCouldn't unstick application from the launcher.");
                }
            }
        }
    }
    Clean();

    // Free memory.
    delete ui;
    delete item;
    delete scene;
}

// String to char.
char *UbuntuInstaller::ToChar(QString toChar) {
    QByteArray uchar = toChar.toLatin1();
    return strdup(uchar.data());
}

void UbuntuInstaller::Error(QString message) { QMessageBox::critical(this_widget, "Error", message); }

// Generate a script file.
bool UbuntuInstaller::MakeScript(QString filename, QString arg) {
    QFile inst(filename);
    if (inst.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&inst);
        out << "#!/bin/sh\n";
        out << arg;
        inst.close();

        if (QProcess::execute("chmod", QStringList() << "+x" << filename) != 0) {
            Error("\nCouldn't set " + filename + " script to executable.");
            inst.remove();
            return false;
        }
        return true;
    } else Error("\nCouldn't create " + filename + " script.");
    return false;
}

// Generate a script file, using a whole list to write out.
bool UbuntuInstaller::MakeScript(QString filename, QStringList arglist) {
    QFile inst(filename);
    if (inst.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&inst);
        out << "#!/bin/sh\n";
        foreach (QString line, arglist) out << line;
        inst.close();

        if (QProcess::execute("chmod", QStringList() << "+x" << filename) != 0) {
            Error("\nCouldn't set " + filename + " script to executable.");
            inst.remove();
            return false;
        }
        return true;
    } else Error("\nCouldn't create " + filename + " script.");
    return false;
}

// Generate, Execute and Delete a script at once.
bool UbuntuInstaller::OnFlyScript(QString filename, QString arg) {
    if (MakeScript(filename, arg)) {
        if (QProcess::execute("sh", QStringList() << ("./" + filename)) == 0) {
            if (!QFile::remove(filename)) return false;
        } else return false;
    } else return false;
    return true;
}

// Generate, Execute and Delete a script at once. But using a whole list to write out.
bool UbuntuInstaller::OnFlyScript(QString filename, QStringList arglist) {
    if (MakeScript(filename, arglist)) {
        if (QProcess::execute(filename, QStringList() << ("./" + filename)) == 0) {
            if (!QFile::remove(filename)) return false;
        } else return false;
    } else return false;
    return true;
}

// Run a external app or shell command, catch the output and return it to the caller.
bool UbuntuInstaller::Run(QString command, QString arguments, QString *result) {
    // Initialize a new Process.
    bool _result = false;
    QProcess   *proc = new QProcess();

    // Merge Channels.
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Setup Signals, aka Events.
    connect(proc, &QProcess::started, [proc, &arguments]()->void{ proc->write((arguments + '\n').toLatin1()); });
    connect(proc, &QProcess::readyReadStandardOutput, [proc, &result]()->void{ result->append(proc->readAllStandardOutput()); });
    connect(proc, &QProcess::readyReadStandardError, [proc, &result, &_result]()->void{ result->append(proc->readAllStandardError()); _result = true; });
    connect(proc, &QProcess::errorOccurred, [&result, &command, &arguments]()->void{ Error("\nAn Error Occured while executing:\n" + command + " " + arguments); result->clear(); });

    // Start the Process.
    proc->start(command);

    // Check if Process could be started.
    if (!proc->waitForStarted()){
        Error("\nCan't start:\n" + command + " " + arguments);
        proc->kill();
        return false;
    }

    // Check if process timed out.
    if (!proc->waitForFinished()) {
        Error("\nTimmed out:\n" + command + " " + arguments);
        proc->kill();
        return false;
    }

    if (_result) return false;
    return true;
}

// Run a external app or shell command, catch the output and return it to the caller.
bool UbuntuInstaller::Run(QString command, QStringList arguments, QString *result) {
    // Initialize a new Process.
    bool _result = false;
    QProcess   *proc = new QProcess();

    // Merge Channels.
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Setup Signals, aka Events.
    connect(proc, &QProcess::readyReadStandardOutput, [proc, &result]()->void{result->append(proc->readAllStandardOutput()); });
    connect(proc, &QProcess::readyReadStandardError, [proc, &result, &_result]()->void{result->append(proc->readAllStandardError()); _result = true; });
    connect(proc, &QProcess::errorOccurred, [&result, &command, &arguments]()->void{Error("\nAn Error Occured while executing: " + command + " " + arguments.join(" ")); result->clear(); });

    // Start the Process.
    proc->start(command, arguments);

    // Check if Process could be started.
    if (!proc->waitForStarted()){
        Error("\nCan't start:\n" + command + " " + arguments.join(" "));
        proc->kill();
        return false;
    }

    // Check if process timed out.
    if (!proc->waitForFinished()) {
        Error("\nTimmed out:\n" + command + " " + arguments.join(" "));
        proc->kill();
        return false;
    }

    if (_result) return false;
    return true;
}

// Run a external app or shell command, catch the output and return it to the caller.
QString UbuntuInstaller::Run(QString command, QString arguments) {
    // Initialize a new Process and a result string to store.
    QString    result;
    QProcess   *proc = new QProcess();

    // Merge Channels.
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Setup Signals, aka Events.
    connect(proc, &QProcess::started, [proc, &arguments]()->void{proc->write((arguments + '\n').toLatin1()); });
    connect(proc, &QProcess::readyReadStandardOutput, [proc, &result]()->void{result += proc->readAllStandardOutput(); });
    connect(proc, &QProcess::readyReadStandardError, [proc, &result]()->void{result += proc->readAllStandardError(); });
    connect(proc, &QProcess::errorOccurred, [&result, &command, &arguments]()->void{Error("An Error Occured while executing: " + command + " " + arguments); result.clear(); });

    // Start the Process.
    proc->start(command);

    // Check if Process could be started.
    if (!proc->waitForStarted()){
        Error("\nCan't start:\n" + command + " " + arguments);
        proc->kill();
    }

    // Check if process timed out.
    if (!proc->waitForFinished()) {
        Error("\nTimmed out:\n" + command + " " + arguments);
        proc->kill();
    }
    return result;
}

// Run a external app or shell command, catch the output and return it to the caller.
QString UbuntuInstaller::Run(QString command, QStringList arguments) {
    // Initialize a new Process and a result string to store.
    QString    result;
    QProcess   *proc = new QProcess();

    // Merge Channels.
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Setup Signals, aka Events.
    connect(proc, &QProcess::readyReadStandardOutput, [proc, &result]()->void{result += proc->readAllStandardOutput();});
    connect(proc, &QProcess::readyReadStandardError, [proc, &result]()->void{result += proc->readAllStandardError();});
    connect(proc, &QProcess::errorOccurred, [&result, &command, &arguments]()->void{Error("An Error Occured while executing: " + command + " " + arguments.join(" ")); result.clear();});

    // Start the Process.
    proc->start(command, arguments);

    // Check if Process could be started.
    if (!proc->waitForStarted()){
        Error("\nCan't start:\n" + command + " " + arguments.join(" "));
        proc->kill();
    }

    // Check if process timed out.
    if (!proc->waitForFinished()) {
        Error("\nTimmed out:\n" + command + " " + arguments.join(" "));
        proc->kill();
    }
    return result;
}

void UbuntuInstaller::on_actionThis_thing_triggered() {
    QString about = "\nThis is a simple and easy Installer for ";
    about += "Ubuntu Applications where the normal .deb or Software Installer is not enough.\n";
    about += "But should also work, with some tiny tweaks, on other linux distros.\n\n";
    about += "Builded with Qt 5.10\n\nby cfwprpht";
    QMessageBox::information(this, "About", about);
}

void UbuntuInstaller::on_buttonOpen_clicked() {
    QString install = QFileDialog::getExistingDirectory(this, tr("Open Directory to Install"),
                                                    home,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if (install.length() > 0) {
        if (install.startsWith(home)) ui->textBox->setText(install);
        else Error("We need to install to a non root location.");
    }
}

/*void UbuntuInstaller::on_buttonInstall_clicked() {
    if (ui->textBox->text().length() > 0) {
        if (ui->textBox->text().startsWith(home)) {
            if (MakeWorkingDir()) {                
                QDir current = QDir::current();
                QDir::setCurrent(work_dir);
                SUDialog sudialog(this);
                sudialog.show();
                if (sudialog.exec() == QDialog::Accepted) {
                    if (SUDialog::supassword.length() > 0) {
                        alldone = false;
                        installTo = ui->textBox->text() + "/" + appname + "/";
                        if (CheckInstallClean()) {
                            chmodx << home + "/.local/share/applications/" + appname + ".desktop";
                            chmodx << home + "/.local/share/applications/" + appname + ".uninstall.desktop";
                            chmodx << installTo + appname;
                            chmodx << installTo + "uninstall.sh";
                            if (Unzip()) {
                                QDir instdir(work_dir + "/" + appname);
                                if (instdir.exists()) {
                                    if (MakeSudoSymLink()) {
                                        if (MakeUninstallScript()) {
                                            if (MakeDesktopEntry()) {
                                                if (MakeUninstallDesktopEntry()) {
                                                    if (Install(instdir.path(), installTo)) {
                                                        QDir::setCurrent(current.path());
                                                        if (CheckInstallOk()) {
                                                            if (Clean()) {
                                                                QString error;
                                                                if (!Run("sh", QStringList() << "source" << "/etc/profile.d/supwd.sh", &error)) Error("\nCouldn't start supwd bootup script.\nPlease log in and out to trigger it.\n\n" + error);

                                                                QMessageBox::information(this, "Installer", "\nAll done.");
                                                                alldone = true;
                                                            } else QMessageBox::information(this, "Installer", "\nCouldn't clean up the temporary Directory: \n\n" + home + "/" + temp_name + "\n\n" + "else...\nAll done.");
                                                        } else Error("\nCouldn't install all files.");
                                                    } else Error("\nCouldn't copy all files and folders.");
                                                } else Error("\nCouldn't create uninstall desktop entry.");
                                            } else Error("\nCouldn't create desktop entry.");
                                        } else Error("\nCouldn't create uninstallation script.");
                                    } else Error("\nCouldn't create sudo login script.");
                                } else Error("\nInstallation Directorie does not exist.");
                            } else Error("\nCouldn't extract all files and folders.");
                        } else QMessageBox::information(this, "Installer", "\nInstallation canceled.");
                    } else Error("\nThat's to short.\n\nPlease enter your su pass other wise " + appname + " can't run.");
                } else Error("\nThe Installed app requires Super User state.\n\nPlease enter your su pass other wise " + appname + " can't run.");
            } else Error("Couldn't create the temporary working folder.");
        } else Error("We need to install to a non root location.");
    } else Error("\nPlease enter a valid path to install.");
}*/

void UbuntuInstaller::on_buttonInstall_clicked() {
    if (ui->textBox->text().length() > 0) {
        if (ui->textBox->text().startsWith(home)) {
            if (MakeWorkingDir()) {
                QDir current = QDir::current();
                QDir::setCurrent(work_dir);
                alldone = false;
                installTo = ui->textBox->text() + "/" + appname + "/";
                if (CheckInstallClean()) {
                    chmodx << home + "/.local/share/applications/" + appname + ".desktop";
                    chmodx << home + "/.local/share/applications/" + appname + ".uninstall.desktop";
                    chmodx << installTo + appname;
                    chmodx << installTo + "uninstall.sh";
                    if (Unzip()) {
                        QDir instdir(work_dir + "/" + appname);
                        if (instdir.exists()) {
                            if (MakeSudoSymLink()) {
                                if (MakeUninstallScript()) {
                                    if (MakeDesktopEntry()) {
                                        if (MakeUninstallDesktopEntry()) {
                                            if (Install(instdir.path(), installTo)) {
                                                QDir::setCurrent(current.path());
                                                if (CheckInstallOk()) {
                                                    if (Clean()) {
                                                        QString error;
                                                        if (!Run("sh", QStringList() << "source" << "/etc/profile.d/supwd.sh", &error)) Error("\nCouldn't start supwd bootup script.\nPlease log in and out to trigger it.\n\n" + error);

                                                        QMessageBox::information(this, "Installer", "\nAll done.");
                                                        alldone = true;
                                                    } else QMessageBox::information(this, "Installer", "\nCouldn't clean up the temporary Directory: \n\n" + home + "/" + temp_name + "\n\n" + "else...\nAll done.");
                                                } else Error("\nCouldn't install all files.");
                                            } else Error("\nCouldn't copy all files and folders.");
                                        } else Error("\nCouldn't create uninstall desktop entry.");
                                    } else Error("\nCouldn't create desktop entry.");
                                } else Error("\nCouldn't create uninstallation script.");
                            } else Error("\nCouldn't create sudo login script.");
                        } else Error("\nInstallation Directorie does not exist.");
                    } else Error("\nCouldn't extract all files and folders.");
                } else QMessageBox::information(this, "Installer", "\nInstallation canceled.");
            } else Error("Couldn't create the temporary working folder.");
        } else Error("We need to install to a non root location.");
    } else Error("\nPlease enter a valid path to install.");
}

void UbuntuInstaller::on_buttonAbort_clicked() {
    if (QMessageBox::question(this, "Abort ?", "Abort the Installtion and close App ?") == QMessageBox::StandardButton::Yes) {
        this->close();
    }
}

void UbuntuInstaller::on_label_linkActivated(const QString &link) { QDesktopServices::openUrl(link); }

// Copy files and folders recursive.
bool UbuntuInstaller::CopyDirectory(QString source, QString destination) {
    // First check if source dir exists and if it is a dir.
    QFileInfo *info = new QFileInfo(source);
    if (!info->exists() || !info->isDir()) {
        if (!info->exists()) Error("\nSource Directory does not exist !");
        else Error("\nSource path is not a Directory !");
        delete info;
        return false;
    }
    delete info;

    // Now check if destination exists and if so if it is a directory.
    info = new QFileInfo(destination);
    if (info->exists() && !info->isDir()) {
        Error("\nDestination path is not a Directory !");
        delete info;
        return false;
    }
    delete info;

    // Set up the paths and get a list of all files and folders of the source dir.
    QDir src(source);
    QDir dest(destination);
    QFileInfoList entrys = src.entryInfoList();

    // Check if destination dir exists and create if not.
    if (!dest.exists()) {
        if (!dest.mkpath(".")) {
            Error("\nCouldn't create folder: \n\n" + dest.path());
            return false;
        }
    }

    // Copy Files and folders recursive.
    foreach (QFileInfo entry, entrys) {
        if (entry.isDir() && (entry.fileName().contains(".") || entry.fileName().contains(".."))) continue;

        if (entry.isFile()) {
            QString toCopy = destination;
            if (!toCopy.endsWith("/")) toCopy += "/";
            toCopy += entry.fileName();
            if (!QFile::copy(entry.filePath(), toCopy)) {
                Error("\nCouldn't copy file: \n\n" + entry.filePath() + "\nto\n" + toCopy);
                return false;
            }
        } else if (entry.isDir()) {
            QString toCopy = destination;
            if (!toCopy.endsWith("/")) toCopy += "/";
            toCopy += entry.fileName();
            if (!CopyDirectory(entry.filePath(), toCopy)) {
                Error("\nCouldn't copy directory: \n\n" + entry.filePath() + "\nto\n" + toCopy);
                return false;
            }
        }
    }
    return true;
}

// Check the installation.
bool UbuntuInstaller::CheckInstallOk(void) {
    QDir check(installTo);
    if (!check.exists()) return false;

    QFile *checkf = new QFile(home + "/.local/share/icons/" + appname + ".png");
    if (!checkf->exists()) { delete checkf; return false; }

    delete checkf;
    checkf = new QFile(home + "/.local/share/applications/" + appname + ".desktop");
    if (!checkf->exists()) { delete checkf; return false; }

    delete checkf;
    checkf = new QFile(home + "/.local/share/applications/" + appname + ".uninstall.desktop");
    if (!checkf->exists()) { delete checkf; return false; }

    delete checkf;
    QString appdb;
    if (ui->checkBoxLauncher->checkState() == Qt::Checked) {
        if (Run("gsettings", QStringList() << "get" << "com.canonical.Unity.Launcher" << "favorites", &appdb)) {
            if (!appdb.contains(" 'application://" + appname + ".desktop")) return false;
        } else { delete checkf; return false; }
    }
    return true;
}

// Check the installation.
bool UbuntuInstaller::CheckInstallClean(void) {
    QDir check(installTo);
    if (check.exists()) {
        Error("\nThe Application is already installed !\nIf you want to remove it, please use the uninstall desktop entry, which you can find within Unity.");
        return false;
    }

    int count = 0;
    QString found;
    QFile *checkf = new QFile(home + "/.local/share/icons/" + appname + ".png");
    if (checkf->exists()) {
        count++;
        found += home + "/.local/share/icons/" + appname + ".png\n";
    }

    delete checkf;
    checkf = new QFile(home + "/.local/share/applications/" + appname + ".desktop");
    if (checkf->exists()) {
        count++;
        found += home + "/.local/share/applications/" + appname + ".desktop\n";
    }

    delete checkf;
    checkf = new QFile(home + "/.local/share/applications/" + appname + ".uninstall.desktop");
    if (checkf->exists()) {
        count++;
        found += home + "/.local/share/applications/" + appname + ".uninstall.desktop\n";
    }

    delete checkf;
    /*checkf = new QFile("/etc/profile.d/supwd.sh");
    if (checkf->exists()) {
        count++;
        found += "/etc/profile.d/supwd.sh\n";
    }*/

    QString appdb;
    if (ui->checkBoxLauncher->checkState() == Qt::Checked) {
        if (Run("gsettings", QStringList() << "get" << "com.canonical.Unity.Launcher" << "favorites", &appdb)) {
            if (appdb.contains(" 'application://" + appname + ".desktop")) {
                count++;
                found += "'application://" + appname + ".desktop";
            }
        }
    }

    if (count > 0) {
        Error("\nFound thoes " + QString::number(count) + " left file(s)\n\nEither they are from a other installation or they're just left corpses:\n" + found);
        if (QMessageBox::question(this, "What to do ?", "\nCancel Installation\nOr\nClean dead corpses and continue ?") == QMessageBox::Yes) {
            QStringList err;
            QStringList list = found.split("\n");

            foreach (QString line, list) {
                if (!line.contains("/etc/") && !line.contains("'application://") && !line.isNull()) {
                    checkf = new QFile(line);
                    if (!checkf->remove()) err << line;
                    delete checkf;
                } /*else if (line.contains("/etc/profile.d/supwd.sh")) {
                    if (!OnFlyScript("uninstall_su.sh", "echo " + SUDialog::supassword + " | sudo rm -f /etc/profile.d/supwd.sh"))
                        err << line;
                }*/ else if (line.contains("'application://")) {
                    QString appdb, new_appdb, error;
                    if (Run("gsettings", "get com.canonical.Unity.Launcher favorites", &appdb)) {
                        if(Run("/bin/sh", "-c \"echo " + appdb + " | sed s/'application:""\"/""\"/" + appname + ".desktop', //\"", &new_appdb)) {
                            if (!Run("gsettings", "set com.canonical.Unity.Launcher favorites " + new_appdb, &error)) {
                                err << "Right click on the starter within the Launcher >> 'Unpin app from Launcher'";
                                return false;
                            } else err << "Right click on the starter within the Launcher >> 'Unpin app from Launcher'";
                        } else err << "Right click on the starter within the Launcher >> 'Unpin app from Launcher'";
                    } else err << "Right click on the starter within the Launcher >> 'Unpin app from Launcher'";
                }
            }

            if (err.length() > 0) {
                Error("\nCouldn't completely clean up.\nPlease try to remove thoes files on your own:\n" + err.join(""));
                return false;
            }
        } else return false;
    }
    return true;
}

// Clean up the temporary folder and installer files.
bool UbuntuInstaller::Clean(void) {
    QDir temp(work_dir);
    if (temp.exists())
        if (temp.removeRecursively()) return true;
    return false;
}

// Unpack a Zip file, using zlib library and "" C++ wrapper.
bool UbuntuInstaller::Unzip(void) {
    QFile::copy(":/dummy_app.zip", appname + ".zip");
    if (QProcess::execute("unzip", QStringList() << appname + ".zip") != 0) return false;
    return true;
}

// Create the temporary work folder.
bool UbuntuInstaller::MakeWorkingDir(void) {
    QDir temp(work_dir);
    if (temp.exists()) {
        if (!temp.removeRecursively()) return false;
    }
    if (!temp.mkpath(".")) return false;    
    return true;
}

bool UbuntuInstaller::MakeDesktopEntry(void) {
    QFile deskentry(appname + ".desktop");
    if (deskentry.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&deskentry);

        int index;
        foreach (QString entry, desktop) {
            if (entry.contains("$HOME")) {
                index = entry.indexOf("$HOME");
                entry.replace(index, tr("$HOME").length(), home);
            } else if (entry.contains("$INSP")) {
                index = entry.indexOf("$INSP");
                entry.replace(index, tr("$INSP").length(), installTo);
            }
            out << entry;
        }
        deskentry.close();
        return true;
    }
    return false;
}

bool UbuntuInstaller::MakeUninstallDesktopEntry(void) {
    QFile uideskentry(appname + ".uninstall.desktop");
    if (uideskentry.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&uideskentry);

        int index;
        foreach (QString entry, uidesktop) {
            if (entry.contains("$HOME")) {
                index = entry.indexOf("$HOME");
                entry.replace(index, tr("$HOME").length(), home);
            } else if (entry.contains("$INSP")) {
                index = entry.indexOf("$INSP");
                entry.replace(index, tr("$INSP").length(), installTo);
            }
            out << entry;
        }
        uideskentry.close();
        return true;
    }
    return false;
}

bool UbuntuInstaller::Install(QString src, QString dest) {
    // Install app icon.
    if (!QFile::copy(":/app.png", home + "/.local/share/icons/" + appname + ".png")) {
        Error("\nCouldn't install application icon.");
        return false;
    }

    // Install desktop entrys.
    if (!QFile::copy("ps4-exploit-host.desktop", home + "/.local/share/applications/" + appname + ".desktop")) {
        Error("\nCouldn't install desktop entry.");
        return false;
    }
    if (!QFile::copy("ps4-exploit-host.uninstall.desktop", home + "/.local/share/applications/" + appname + ".uninstall.desktop")) {
        Error("\nCouldn't install uninstall desktop entry.");
        return false;
    }

    // Install SU bootup script.
    /*if (!OnFlyScript("install_su.sh", ("echo " + SUDialog::supassword + " | sudo -S cp chips.sh /etc/profile.d/supwd.sh"))) {
        Error("\nCouldn't create sudo bootup script.");
        return false;
    }*/

    // Copy uninstall script to app folder.
    if (!QFile::copy("uninstall.sh", work_dir + "/" + appname + "/uninstall.sh")) {
        Error("\nCouldn't copy uninstall script to app folder.");
        return false;
    }

    // Copy app folder to finall destination.
    if (!CopyDirectory(src, dest)) {
        Error("\nCouldn't copy all application files.");
        return false;
    }

    // Make majore files executable.
    foreach (QString entry, chmodx) {
        if (QProcess::execute("chmod", QStringList() << "+x" << entry) != 0) {
            Error("\nCouldn't set " + entry + " to executable.");
            return false;
        }
    }

    // Set icon to write too for the current user.
    if (QProcess::execute("chmod", QStringList() << "+w" << (home + "/.local/share/icons/" + appname + ".png")) != 0) {
        Error("\nCouldn't set app icon to write for the current user.");
        return false;
    }

    // Pinn desktop shortcut to the launcher.
    if (ui->checkBoxLauncher->checkState() == Qt::Checked) {
        QString appdb, error;
        if (Run("gsettings", QStringList() << "get" << "com.canonical.Unity.Launcher" << "favorites", &appdb)) {
            QStringList splitted = appdb.split(",");
            int indexer, count;
            indexer = count = 0;
            foreach (QString line, splitted) {
                if (line.contains("'application://")) indexer++;
            }

            QStringList added;
            foreach (QString line, splitted) {
                added << line;
                if (count == indexer) added << " 'application://" + appname + ".desktop'";
                count++;
            }

            if (!Run("gsettings", QStringList() << "set" << "com.canonical.Unity.Launcher" << "favorites" << added.join(","), &error)) {
                Error("\nCouldn't stick application to the launcher.");
                return false;
            }
        }
    }
    return true;
}

bool UbuntuInstaller::MakeUninstallScript(void) {
    QStringList out;
    //out << "echo $SUPWD | sudo -S rm -f /etc/profile.d/chips.sh";
    out << "rm -f \"$HOME\"/.local/share/applications/" + appname + ".desktop\n";
    out << "rm -f \"$HOME\"/.local/share/applications/" + appname + ".uninstall.desktop\n";
    out << "rm -f \"$HOME\"/.local/share/icons/" + appname + ".png\n";
    out << "rm -R " + installTo + "\n\n";

    // Add the unpinn script to the uninstaller.
    if (ui->checkBoxLauncher->checkState() == Qt::Checked) {
        foreach (QString entry, unpinentry) out << entry;
    }

    if (!MakeScript("uninstall.sh", out)) return false;
    return true;
}

bool UbuntuInstaller::MakeSudoSymLink(void) {
    if (!MakeScript("supwd.sh", ("export SUPWD=" + SUDialog::supassword))) return false;
    return true;
}
