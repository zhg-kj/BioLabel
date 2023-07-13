#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QtGui>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    layout = ui->gridLayout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    uploadButton = ui->uploadButton;
    saveGoodButton = ui->saveGoodButton;
    saveBadButton = ui->saveBadButton;
    stitchButton = ui->stitchButton;
    QScrollArea *scrollArea = ui->scrollArea;

    // Set the grid layout as the widget inside the scroll area
    QWidget *gridWidget = new QWidget();
    gridWidget->setLayout(layout);
    scrollArea->setWidget(gridWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(uploadButton, &QToolButton::clicked, this, &MainWindow::uploadFolder);
    connect(saveGoodButton, &QPushButton::clicked, this, &MainWindow::saveGoodImages);
    connect(saveBadButton, &QPushButton::clicked, this, &MainWindow::saveBadImages);
    connect(ui->goodCheckBox, &QCheckBox::stateChanged, this, &MainWindow::viewGoodImages);
    connect(ui->badCheckBox, &QCheckBox::stateChanged, this, &MainWindow::viewBadImages);
    connect(stitchButton, &QToolButton::clicked, this, &MainWindow::uploadRawFolder);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * This function displays error and success messages in a modal.
 *
 * @author Kai Jun Zhuang
 * @param message The message to be displayed in the QMessageBox.
 */
void MainWindow::showLogMessage(const QString& message)
{
    QMessageBox::information(this, tr("Info"), message);
}

/**
 * This function recursively retrieves all image files with the extensions ".tif" and ".png"
 * from the specified folder and its subfolders.
 *
 * @author Kai Jun Zhuang
 * @param folderPath The path of the folder to search for image files.
 * @param imageFiles [out] A list to store the retrieved image files.
 */
void getAllImageFiles(const QString &folderPath, QFileInfoList &imageFiles) {
    QDir folder(folderPath);
    QFileInfoList files = folder.entryInfoList(QDir::Files);
    for (const QFileInfo &file : files) {
        if (file.suffix() == "tif" || file.suffix() == "png") {
            imageFiles.append(file);
        }
    }
    QFileInfoList subDirs = folder.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QFileInfo &subDir : subDirs) {
        getAllImageFiles(subDir.filePath(), imageFiles);
    }
}

/**
 * This function allows the user to upload a folder of subfolders containing images to be stitched.
 * The stitched images are then displayed as thumbnail buttons in a grid layout in the main window.
 * The user can interact with the buttons to view larger images, mark them as good or bad, and delete them.
 * The function assumes that each subfolder with images that need to be stitched begins with the naming convention "XY".
 * The stitched images are saved to another user-selected folder.
 *
 * @author Kai Jun Zhuang
 */
void MainWindow::uploadFolder()
{
    // Load the Qt Image Formats plugin
    QPluginLoader pluginLoader("imageformats/qtiff.dll");
    QObject *plugin = pluginLoader.instance();
    if (!plugin) {
        qDebug() << "Failed to load Qt Image Formats plugin:" << pluginLoader.errorString();
        return;
    }

    // Open a file dialog to select a folder
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (folderPath.isEmpty())
        return;

    // Get a list of all .tif and .png files in the selected folder
    QFileInfoList imageFiles;
    getAllImageFiles(folderPath, imageFiles);

    // Initialize variables for traversing the gridLayout
    int row = 0;
    int col = 0;
    const int COLUMNS = 5; // Number of columns in the grid

    // Get the current number of rows and columns in the grid layout
    int currentRows = layout->rowCount() - 1;
    int currentCols = layout->columnCount() - 1;

    // Set the row and column indices to the next available position in the grid
    row = currentRows;
    col = currentCols;
    if (col + 1 == COLUMNS) {
        col = 0;
        row++;
    }

    for (const QFileInfo &fileInfo : imageFiles) {
        // Load the image file into a QPixmap
        QPixmap pixmap;
        if (!pixmap.load(fileInfo.absoluteFilePath())) {
            qDebug() << "Failed to load image:" << fileInfo.absoluteFilePath();
            continue;
        }

        // Scale the pixmap to a larger thumbnail size
        pixmap = pixmap.scaled(220, 220, Qt::KeepAspectRatio);

        // Create a QPushButton widget to display the pixmap
        QPushButton *button = new QPushButton();
        button->setIcon(pixmap);
        button->setIconSize(pixmap.size());
        button->setFlat(true);
        button->setAutoFillBackground(true);
        button->setProperty("imagePath", fileInfo.absoluteFilePath());
        button->setCursor(Qt::PointingHandCursor);

        // Set the button's palette to red
        QPalette palette;
        palette.setColor(QPalette::Button, Qt::red);
        button->setPalette(palette);

        // Add the button to the grid layout
        layout->addWidget(button, row, col);

        // Connect the button's clicked signal to a slot
        connect(button, &QPushButton::clicked, [this, button]() {
            // Toggle the border on and off when the button is clicked
            if (button->palette().color(QPalette::Button) == Qt::green) {
                QPalette palette;
                palette.setColor(QPalette::Button, Qt::red);
                button->setPalette(palette);
            } else {
                QPalette palette;
                palette.setColor(QPalette::Button, Qt::green);
                button->setPalette(palette);
            }
        });

        // Create a context menu for the button
        QMenu *contextMenu = new QMenu(this);

        // Actions for the context menu
        QAction *openInNewWindow = new QAction("View larger image", this);
        QAction *deleteButton = new QAction("Delete", this);
        contextMenu->addAction(openInNewWindow);
        contextMenu->addAction(deleteButton);

        // Context Menu Settings
        button->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(button, &QPushButton::customContextMenuRequested, [this, button, contextMenu, pixmap](){
            contextMenu->exec(QCursor::pos());
        });

        // Open In New Window functionality
        connect(openInNewWindow, &QAction::triggered, [this, button, pixmap](){
            // Create a new window to display the image
            QDialog *imageDialog = new QDialog(this);
            imageDialog->setWindowTitle("Image View");
            QLabel *label = new QLabel(imageDialog);
            label->setPixmap(pixmap.scaled(pixmap.width()*4, pixmap.height()*4, Qt::KeepAspectRatio));
            QVBoxLayout *layout = new QVBoxLayout(imageDialog);
            layout->addWidget(label);
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            QPushButton *setGreen = new QPushButton("Mark as Good", imageDialog);
            QPushButton *setRed = new QPushButton("Mark as Bad", imageDialog);
            buttonLayout->addWidget(setGreen);
            buttonLayout->addWidget(setRed);
            layout->addLayout(buttonLayout);

            if(button->palette().color(QPalette::Button) == Qt::green)
                setGreen->setDisabled(true);
            else if(button->palette().color(QPalette::Button) == Qt::red)
                setRed->setDisabled(true);

            connect(setGreen, &QPushButton::clicked, [this, button, imageDialog](){
                QPalette palette;
                palette.setColor(QPalette::Button, Qt::green);
                button->setPalette(palette);
                imageDialog->accept();
            });
            connect(setRed, &QPushButton::clicked, [this, button, imageDialog](){
                QPalette palette;
                palette.setColor(QPalette::Button, Qt::red);
                button->setPalette(palette);
                imageDialog->accept();
            });
            imageDialog->exec();
        });

        // Delete item functionality
        connect(deleteButton, &QAction::triggered, [this, button](){
            layout->removeWidget(button);
            delete button;
        });

        // Increment the column and row indices
        col++;
        if (col == COLUMNS) {
            col = 0;
            row++;
        }
    }
}

/**
 * This function saves the images marked as good by the user.
 * The function prompts the user to select a folder to save the good images.
 * It iterates over the thumbnail buttons in the grid layout and saves the corresponding images
 * with the naming convention "<imageName>_good.tif" to the selected folder.
 *
 * @author Kai Jun Zhuang
 */
void MainWindow::saveGoodImages()
{
    // Open a file dialog to select a folder to save the images to
    QString saveFolderPath = QFileDialog::getExistingDirectory(this, tr("Select Save Folder"), QString());
    if (saveFolderPath.isEmpty()) {
        return;
    }

    // Iterate over all widgets in the grid layout
    for (int i = 0; i < layout->count(); i++) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) {
            continue;
        }

        // Check if the widget is a QPushButton with a green palette
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        if (!button) {
            continue;
        }
        if (button->palette().color(QPalette::Button) != Qt::green) {
            continue;
        }

        // Get the original image file name
        QString imagePath = button->property("imagePath").toString();
        QFileInfo imageFileInfo(imagePath);
        QString imageName = imageFileInfo.baseName();

        // Save the button's icon to a file in the save folder
        QString filePath = saveFolderPath + QDir::separator() + imageName + "_good.tif";
        QPixmap pixmap = button->icon().pixmap(button->iconSize());
        if (!pixmap.save(filePath)) {
            qDebug() << "Failed to save image:" << filePath;
            showLogMessage("Failed to save image: " + filePath);
            continue;
        }
    }
    showLogMessage("Good images saved.");
}

/**
 * This function saves the images marked as bad by the user.
 * The function prompts the user to select a folder to save the bad images.
 * It iterates over the thumbnail buttons in the grid layout and saves the corresponding images
 * with the naming convention "<imageName>_bad.tif" to the selected folder.
 *
 * @author Kai Jun Zhuang
 */
void MainWindow::saveBadImages()
{
    // Open a file dialog to select a folder to save the images to
    QString saveFolderPath = QFileDialog::getExistingDirectory(this, tr("Select Save Folder"), QString());
    if (saveFolderPath.isEmpty()) {
        return;
    }

    // Iterate over all widgets in the grid layout
    for (int i = 0; i < layout->count(); i++) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) {
            continue;
        }

        // Check if the widget is a QPushButton with a green palette
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        if (!button) {
            continue;
        }
        if (button->palette().color(QPalette::Button) != Qt::red) {
            continue;
        }

        // Get the original image file name
        QString imagePath = button->property("imagePath").toString();
        QFileInfo imageFileInfo(imagePath);
        QString imageName = imageFileInfo.baseName();

        // Save the button's icon to a file in the save folder
        QString filePath = saveFolderPath + QDir::separator() + imageName + "_bad.tif";
        QPixmap pixmap = button->icon().pixmap(button->iconSize());
        if (!pixmap.save(filePath)) {
            qDebug() << "Failed to save image:" << filePath;
            showLogMessage("Failed to save image: " + filePath);
            continue;
        }
    }
    showLogMessage("Bad images saved.");
}

/**
 * This function controls the visibility of the thumbnail buttons displaying the good images.
 * When the state of the "View Good Images" checkbox is changed, this function is called.
 * It iterates over the thumbnail buttons in the grid layout and sets their visibility
 * based on the state of the checkbox.
 *
 * @author Kai Jun Zhuang
 * @param state The state of the "View Good Images" checkbox (Qt::Checked or Qt::Unchecked).
 */
void MainWindow::viewGoodImages(int state)
{
    // Iterate over all widgets in the grid layout
    for (int i = 0; i < layout->count(); i++) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) {
            continue;
        }

        // Check if the widget is a QPushButton with a green palette
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        if (!button) {
            continue;
        }
        if (button->palette().color(QPalette::Button) != Qt::green) {
            continue;
        }

        // Set the button's visibility based on the state of the green checkbox
        button->setVisible(state == Qt::Checked);
    }
}


/**
 * This function controls the visibility of the thumbnail buttons displaying the bad images.
 * When the state of the "View Bad Images" checkbox is changed, this function is called.
 * It iterates over the thumbnail buttons in the grid layout and sets their visibility
 * based on the state of the checkbox.
 *
 * @author Kai Jun Zhuang
 * @param state The state of the "View Bad Images" checkbox (Qt::Checked or Qt::Unchecked).
 */
void MainWindow::viewBadImages(int state)
{
    // Iterate over all widgets in the grid layout
    for (int i = 0; i < layout->count(); i++) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) {
            continue;
        }

        // Check if the widget is a QPushButton with a red palette
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        if (!button) {
            continue;
        }
        if (button->palette().color(QPalette::Button) != Qt::red) {
            continue;
        }

        // Set the button's visibility based on the state of the red checkbox
        button->setVisible(state == Qt::Checked);
    }
}

/**
 * This function allows users to upload a folder of subfolders containing images to be stitched
 * and then saves the stitched images to another user selected folder. It assumes that each subfolder with
 * images that need to be stitched begins with the naming convention "XY".
 *
 * It does so by:
 *      1. Prompting the user to choose a folder containing the subfolders with images they want stitched.
 *      2. Prompting the user to choose a folder to save stitched images to.
 *      3. Calling on the stitchFolder helper function on each subfolder in the selected folder.
 *
 * @author Kai Jun Zhuang
 */
void MainWindow::uploadRawFolder()
{
    // Show a file dialog to select the folder containing the images to stitch
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder Containing XY Subfolders"));
    if (folderPath.isEmpty()) {
        qDebug() << "No folder selected. Please choose a folder containing the images you want stitched.";
        showLogMessage("No folder selected. Please choose a folder containing the images you want stitched.");
        return;
    }

    // Show a file dialog to select the folder to save stitched images to
    QString savePath = QFileDialog::getExistingDirectory(this, tr("Select Where to Save Stitched Images"));
    if (savePath.isEmpty()) {
        qDebug() << "No folder selected. Please choose a folder to save all the stitched images to.";
        showLogMessage("No folder selected. Please choose a folder to save all the stitched images to.");
        return;
    }

    // Get all subfolders in folderPath
    QDir folder(folderPath);
    QStringList subfolders = folder.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Filter through each subfolder and only call stitchFolder on the ones including XY
    foreach (QString subfolder, subfolders)
    {
        if (subfolder.contains("XY")) {
            QString path = folderPath + "/" + subfolder;

            stitchFolder(path, savePath, subfolder.replace(0, 2, "A"));
        }
    }
    showLogMessage("Stitching complete.");
}

/**
 * This function is a helper function for uploadRawFolder. It takes in a subfolder, sorts images into
 * appropriate lists, and then calls the stitchImage helper function on the images. It assumes that the
 * subfolder passed to it contains 4 channels with naming conventions CH1, CH2, CH3, and CH4, and that
 * there exists an overlay of all channels with naming convention Overlay.
 *
 * @author Kai Jun Zhuang
 * @param folderPath The path to the subfolder passed in uploadRawFolder.
 * @param savePath The path to save stitched images to.
 * @param fileName The name of the subfolder to be used in naming the stitched images.
 */
void MainWindow::stitchFolder(QString folderPath, QString savePath, QString fileName)
{
    // Get a list of all .tif files in the selected folder
    QDir folder(folderPath);
    QStringList filters;
    filters << "*.tif";
    folder.setNameFilters(filters);
    QStringList tifFiles = folder.entryList(QDir::Files);

    // Initialize lists
    QStringList ch1;
    QStringList ch2;
    QStringList ch3;
    QStringList ch4;
    QStringList overlay;

    // Add images for channels and overlay into respective lists
    foreach (QString file, tifFiles) {
        if (file.contains("CH1")) {
            ch1.append(folderPath + "/" + file);
        } else if (file.contains("CH2")) {
            ch2.append(folderPath + "/" + file);
        } else if (file.contains("CH3")) {
            ch3.append(folderPath + "/" + file);
        } else if (file.contains("CH4")) {
            ch4.append(folderPath + "/" + file);
        } else if (file.contains("Overlay")) {
            overlay.append(folderPath + "/" + file);
        }
    }

    // Stitch images in each of the lists
    stitchImages(ch1, savePath, fileName + "_CH1");
    stitchImages(ch2, savePath, fileName + "_CH2");
    stitchImages(ch3, savePath, fileName + "_CH3");
    stitchImages(ch4, savePath, fileName + "_CH4");
    stitchImages(overlay, savePath, fileName + "_Overlay");
    showLogMessage("Stitched images for " + fileName + " saved to " + savePath);
}

/**
 * This function is a helper function for the stitchFolder function. It takes a list of images,
 * ensures that there are 9, 16, 25 images, stitches the images together, and then saves the images to
 * the savePath.
 *
 * @author Kai Jun Zhuang
 * @param fileNames A list of the file paths to each image.
 * @param savePath The path to save stitched images to.
 * @param fileName The name for the saved image.
 */
void MainWindow::stitchImages(QStringList fileNames, QString savePath, QString fileName)
{
    // Decide which stitch to perform
    if (fileNames.size() == 9) {
        stitch3x3(fileNames, savePath, fileName);
    } else if (fileNames.size() == 16) {
        stitch4x4(fileNames, savePath, fileName);
    } else if (fileNames.size() == 25) {
        stitch5x5(fileNames, savePath, fileName);
    } else {
        // The number of images is not related to a possible stitch.
        qDebug() << "Wrong number of images for " + fileName + " please ensure there are exactly 9, 16, or 25 images to complete a stitch.";
        showLogMessage("Wrong number of images for " + fileName + ". Please ensure there are exactly 9, 16, or 25 images to complete a stitch.");
        return;
    }
}

void MainWindow::stitch3x3(QStringList fileNames, QString savePath, QString fileName) {
    // Check if there are 9 images in the fileNames list, if not return an error to console
    if (fileNames.size() == 9) {
        // Load the images
        QImage image1(fileNames[0]);
        QImage image2(fileNames[1]);
        QImage image3(fileNames[2]);
        // Account for how files are ordered
        QImage image4(fileNames[5]);
        QImage image5(fileNames[4]);
        // Account for how files are ordered
        QImage image6(fileNames[3]);
        QImage image7(fileNames[6]);
        QImage image8(fileNames[7]);
        QImage image9(fileNames[8]);

        // Create a QPixmap to paint on
        QPixmap pixmap(image1.width() * 3 - 578, image1.height() * 3 - 432);

        // Create a QPainter to draw on the pixmap
        QPainter painter(&pixmap);

        // Draw the images onto the pixmap
        painter.drawImage(0, 0, image1);
        painter.drawImage(image1.width() - 289, 0, image2);
        painter.drawImage(image1.width() * 2 - 578, 0, image3);
        painter.drawImage(0, image1.height() - 216, image4);
        painter.drawImage(image1.width() - 289, image1.height() - 216, image5);
        painter.drawImage(image1.width() * 2 - 578, image1.height() - 216, image6);
        painter.drawImage(0, image1.height() * 2 - 432, image7);
        painter.drawImage(image1.width() - 289, image1.height() * 2 - 432, image8);
        painter.drawImage(image1.width() * 2 - 578, image1.height() * 2 - 432, image9);

        // Convert the pixmap to an image
        QImage image = pixmap.toImage();

        // Show a file dialog to select the output file
        QString filePath = savePath + "/" + fileName + ".png";

        // Save the image to the selected file
        image.save(filePath);
    } else {
        // There were fewer/more than 9 images for the respective channel so log an error message
        qDebug() << "Missing images for " + fileName + " please ensure there are exactly 9 images to complete the stitch.";
        showLogMessage("Missing images for " + fileName + ". Please ensure there are exactly 9 images to complete the stitch.");
        return;
    }
}

void MainWindow::stitch4x4(QStringList fileNames, QString savePath, QString fileName) {
    // Check if there are 16 images in the fileNames list, if not return an error to console
    if (fileNames.size() == 16) {
        // Load the images
        QImage image1(fileNames[0]);
        QImage image2(fileNames[1]);
        QImage image3(fileNames[2]);
        // Account for how files are ordered
        QImage image4(fileNames[5]);
        QImage image5(fileNames[4]);
        // Account for how files are ordered
        QImage image6(fileNames[3]);
        QImage image7(fileNames[6]);
        QImage image8(fileNames[7]);
        QImage image9(fileNames[8]);

        // Create a QPixmap to paint on
        QPixmap pixmap(image1.width() * 3 - 578, image1.height() * 3 - 432);

        // Create a QPainter to draw on the pixmap
        QPainter painter(&pixmap);

        // Draw the images onto the pixmap
        painter.drawImage(0, 0, image1);
        painter.drawImage(image1.width() - 289, 0, image2);
        painter.drawImage(image1.width() * 2 - 578, 0, image3);
        painter.drawImage(0, image1.height() - 216, image4);
        painter.drawImage(image1.width() - 289, image1.height() - 216, image5);
        painter.drawImage(image1.width() * 2 - 578, image1.height() - 216, image6);
        painter.drawImage(0, image1.height() * 2 - 432, image7);
        painter.drawImage(image1.width() - 289, image1.height() * 2 - 432, image8);
        painter.drawImage(image1.width() * 2 - 578, image1.height() * 2 - 432, image9);

        // Convert the pixmap to an image
        QImage image = pixmap.toImage();

        // Show a file dialog to select the output file
        QString filePath = savePath + "/" + fileName + ".png";

        // Save the image to the selected file
        image.save(filePath);
    } else {
        // There were fewer/more than 16 images for the respective channel so log an error message
        qDebug() << "Missing images for " + fileName + " please ensure there are exactly 16 images to complete the stitch.";
        showLogMessage("Missing images for " + fileName + ". Please ensure there are exactly 16 images to complete the stitch.");
        return;
    }
}

void MainWindow::stitch5x5(QStringList fileNames, QString savePath, QString fileName) {
    // Check if there are 25 images in the fileNames list, if not return an error to console
    if (fileNames.size() == 25) {
        // Load the images
        QImage image1(fileNames[0]);
        QImage image2(fileNames[1]);
        QImage image3(fileNames[2]);
        // Account for how files are ordered
        QImage image4(fileNames[5]);
        QImage image5(fileNames[4]);
        // Account for how files are ordered
        QImage image6(fileNames[3]);
        QImage image7(fileNames[6]);
        QImage image8(fileNames[7]);
        QImage image9(fileNames[8]);

        // Create a QPixmap to paint on
        QPixmap pixmap(image1.width() * 3 - 578, image1.height() * 3 - 432);

        // Create a QPainter to draw on the pixmap
        QPainter painter(&pixmap);

        // Draw the images onto the pixmap
        painter.drawImage(0, 0, image1);
        painter.drawImage(image1.width() - 289, 0, image2);
        painter.drawImage(image1.width() * 2 - 578, 0, image3);
        painter.drawImage(0, image1.height() - 216, image4);
        painter.drawImage(image1.width() - 289, image1.height() - 216, image5);
        painter.drawImage(image1.width() * 2 - 578, image1.height() - 216, image6);
        painter.drawImage(0, image1.height() * 2 - 432, image7);
        painter.drawImage(image1.width() - 289, image1.height() * 2 - 432, image8);
        painter.drawImage(image1.width() * 2 - 578, image1.height() * 2 - 432, image9);

        // Convert the pixmap to an image
        QImage image = pixmap.toImage();

        // Show a file dialog to select the output file
        QString filePath = savePath + "/" + fileName + ".png";

        // Save the image to the selected file
        image.save(filePath);
    } else {
        // There were fewer/more than 25 images for the respective channel so log an error message
        qDebug() << "Missing images for " + fileName + " please ensure there are exactly 25 images to complete the stitch.";
        showLogMessage("Missing images for " + fileName + ". Please ensure there are exactly 25 images to complete the stitch.");
        return;
    }
}

