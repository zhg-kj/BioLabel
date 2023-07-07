#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QtGui>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QGridLayout *layout;
    QPushButton *uploadButton;
    QPushButton *saveGoodButton;
    QPushButton *saveBadButton;
    QListWidget *imageList;
    QMenu *fileMenu;
    QStringList tifFiles;
    QStringList ch1;
    QStringList ch2;
    QStringList ch3;
    QStringList ch4;
    QStringList overlay;

private slots:
    void showLogMessage(const QString& message);
    void uploadFolder();
    void saveGoodImages();
    void saveBadImages();
    void viewGoodImages(int);
    void viewBadImages(int);
    void uploadRawFolder();
    void stitchFolder(QString folderPath, QString savePath, QString fileName);
    void stitchImages(QStringList fileNames, QString savePath, QString fileName);
    void stitch3x3(QStringList fileNames, QString savePath, QString fileName);
    void stitch4x4(QStringList fileNames, QString savePath, QString fileName);
    void stitch5x5(QStringList fileNames, QString savePath, QString fileName);
};
#endif // MAINWINDOW_H
