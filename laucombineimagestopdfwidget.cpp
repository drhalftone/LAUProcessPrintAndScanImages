#include "laucombineimagestopdfwidget.h"
#include <QProgressDialog>
#include <QStandardPaths>
#include <QFileDialog>
#include <QPainter>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUCombineImagesToPDFLabel::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);

    // DRAW THE BACKGROUND AS A WHITE FIELD WITH BLACK BORDER
    painter.setBrush(QBrush(QColor(196, 196, 196), Qt::SolidPattern));
    painter.setPen(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
    painter.drawRect(QRect(0, 0, width(), height()));

    // DRAW PAPER
    painter.setBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
    painter.setPen(QPen(QColor(0, 0, 0), 0, Qt::SolidLine));

    double pageRatio = pageHeight / pageWidth;
    double labelRatio = (double)height() / (double)width();
    double rectHeight, rectWidth, leftEdge, topEdge;
    if (pageRatio > labelRatio) {
        rectHeight = (double)height() * 0.95;
        rectWidth = rectHeight / pageHeight * pageWidth;
        leftEdge = (width() - rectWidth) / 2.0;
        topEdge = (height() - rectHeight) / 2.0;
    } else {
        rectWidth = (double)width() * 0.95;
        rectHeight = rectWidth / pageWidth * pageHeight;
        leftEdge = (width() - rectWidth) / 2.0;
        topEdge = (height() - rectHeight) / 2.0;
    }

    // SET THE TRANSFORM FOR PAGE COORDINATES TO LABEL COORDINATES
    QTransform transform;
    transform.translate(leftEdge, topEdge);
    transform.scale(rectWidth, rectHeight);
    painter.setTransform(transform);

    // DRAW THE EDGES OF THE PAPER
    painter.drawRect(QRectF(0.0, 0.0, 1.0, 1.0));

    // DRAW THE EDGES OF THE IMAGE THUMBNAILS
    painter.setBrush(QBrush(QColor(128, 128, 255), Qt::SolidPattern));
    painter.setPen(Qt::NoPen);

    double squareWidth = (pageWidth - 2.0 * leftMargin - gap * (cols - 1)) / cols;
    double squareHeight = (pageHeight - 2.0 * topMargin - gap * (rows - 1)) / rows;
    for (int row = 0; row < rows; row++) {
        double squareTopEdge = topMargin + (squareHeight + gap) * row;
        for (int col = 0; col < cols; col++) {
            double squareLeftEdge = leftMargin + (squareWidth + gap) * col;
            painter.drawRect(QRectF(squareLeftEdge / pageWidth, squareTopEdge / pageHeight, squareWidth / pageWidth, squareHeight / pageHeight));
        }
    }

    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUCombineImagesToPDFWidget::LAUCombineImagesToPDFWidget(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);

    label = new LAUCombineImagesToPDFLabel();
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setMinimumSize(480, 480);

    QWidget *widget = new QWidget();
    widget->setLayout(new QFormLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);

    imageColsSpinBox = new QSpinBox();
    imageColsSpinBox->setFixedWidth(120);
    imageColsSpinBox->setMinimum(1);
    imageColsSpinBox->setMaximum(20);
    imageColsSpinBox->setAlignment(Qt::AlignRight);
    connect(imageColsSpinBox, SIGNAL(valueChanged(int)), label, SLOT(onSetCols(int)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Number of image columns:"), imageColsSpinBox);

    imageRowsSpinBox = new QSpinBox();
    imageRowsSpinBox->setFixedWidth(120);
    imageRowsSpinBox->setMinimum(1);
    imageRowsSpinBox->setMaximum(20);
    imageRowsSpinBox->setAlignment(Qt::AlignRight);
    connect(imageRowsSpinBox, SIGNAL(valueChanged(int)), label, SLOT(onSetRows(int)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Number of image rows:"), imageRowsSpinBox);

    outputResolutionSpinBox = new QSpinBox();
    outputResolutionSpinBox->setFixedWidth(120);
    outputResolutionSpinBox->setMinimum(150);
    outputResolutionSpinBox->setMaximum(2400);
    outputResolutionSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Ouput image resolution (ppi):"), outputResolutionSpinBox);

    pageWidthSpinBox = new QDoubleSpinBox();
    pageWidthSpinBox->setFixedWidth(120);
    pageWidthSpinBox->setMinimum(1.0);
    pageWidthSpinBox->setMaximum(24.0);
    pageWidthSpinBox->setDecimals(4);
    pageWidthSpinBox->setSingleStep(1.0);
    pageWidthSpinBox->setAlignment(Qt::AlignRight);
    connect(pageWidthSpinBox, SIGNAL(valueChanged(double)), label, SLOT(onSetPageWidth(double)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Page width (in):"), pageWidthSpinBox);

    pageHeightSpinBox = new QDoubleSpinBox();
    pageHeightSpinBox->setFixedWidth(120);
    pageHeightSpinBox->setMinimum(1.0);
    pageHeightSpinBox->setMaximum(48.0);
    pageHeightSpinBox->setDecimals(4);
    pageHeightSpinBox->setSingleStep(1.0);
    pageHeightSpinBox->setAlignment(Qt::AlignRight);
    connect(pageHeightSpinBox, SIGNAL(valueChanged(double)), label, SLOT(onSetPageHeight(double)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Page height (in):"), pageHeightSpinBox);

    leftMarginSpinBox = new QDoubleSpinBox();
    leftMarginSpinBox->setFixedWidth(120);
    leftMarginSpinBox->setMinimum(0.0);
    leftMarginSpinBox->setMaximum(2.0);
    leftMarginSpinBox->setDecimals(4);
    leftMarginSpinBox->setSingleStep(0.05);
    leftMarginSpinBox->setAlignment(Qt::AlignRight);
    connect(leftMarginSpinBox, SIGNAL(valueChanged(double)), label, SLOT(onSetLeftMargin(double)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Left/right margins (in):"), leftMarginSpinBox);

    topMarginSpinBox = new QDoubleSpinBox();
    topMarginSpinBox->setFixedWidth(120);
    topMarginSpinBox->setMinimum(0.0);
    topMarginSpinBox->setMaximum(2.0);
    topMarginSpinBox->setDecimals(4);
    topMarginSpinBox->setSingleStep(0.05);
    topMarginSpinBox->setAlignment(Qt::AlignRight);
    connect(topMarginSpinBox, SIGNAL(valueChanged(double)), label, SLOT(onSetTopMargin(double)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Top/bottom margins (in):"), topMarginSpinBox);

    spaceBetweenImages = new QDoubleSpinBox();
    spaceBetweenImages->setFixedWidth(120);
    spaceBetweenImages->setMinimum(0.0);
    spaceBetweenImages->setMaximum(0.5);
    spaceBetweenImages->setSingleStep(0.05);
    spaceBetweenImages->setDecimals(4);
    spaceBetweenImages->setAlignment(Qt::AlignRight);
    connect(spaceBetweenImages, SIGNAL(valueChanged(double)), label, SLOT(onSetGap(double)));
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Gap between images (in):"), spaceBetweenImages);

    simulateNoiseCheckBox = new QComboBox();
    simulateNoiseCheckBox->addItem(QString("NO"));
    simulateNoiseCheckBox->addItem(QString("YES"));
    simulateNoiseCheckBox->setFixedWidth(120);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Simulate Noise:"), simulateNoiseCheckBox);

    this->layout()->addWidget(widget);

    widget = new QWidget();
    widget->setLayout(new QVBoxLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);

    widget->layout()->addWidget(label);
    this->layout()->addWidget(widget);

    QSettings settings;
    imageColsSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::imageColsSpinBox", 6).toInt());
    imageRowsSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::imageRowsSpinBox", 6).toInt());
    outputResolutionSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::outputResolutionSpinBox", 300).toInt());
    pageWidthSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::pageWidthSpinBox", 8.0).toDouble());
    pageHeightSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::pageHeightSpinBox", 11.0).toDouble());
    leftMarginSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::leftMarginSpinBox", 0.5).toDouble());
    topMarginSpinBox->setValue(settings.value("LAUCombineImagesToPDFWidget::topMarginSpin", 0.5).toDouble());
    spaceBetweenImages->setValue(settings.value("LAUCombineImagesToPDFWidget::spaceBetweenImages", 0.1).toDouble());
    simulateNoiseCheckBox->setCurrentText(settings.value("LAUCombineImagesToPDFWidget::simulateNoiseCheckBox", QString("NO")).toString());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUCombineImagesToPDFWidget::~LAUCombineImagesToPDFWidget()
{
    QSettings settings;
    settings.setValue("LAUCombineImagesToPDFWidget::imageColsSpinBox", imageColsSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::imageRowsSpinBox", imageRowsSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::outputResolutionSpinBox", outputResolutionSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::pageWidthSpinBox", pageWidthSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::pageHeightSpinBox", pageHeightSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::leftMarginSpinBox", leftMarginSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::topMarginSpin", topMarginSpinBox->value());
    settings.setValue("LAUCombineImagesToPDFWidget::spaceBetweenImages", spaceBetweenImages->value());
    settings.setValue("LAUCombineImagesToPDFWidget::simulateNoiseCheckBox", simulateNoiseCheckBox->currentText());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUCombineImagesToPDFWidget::processThumbnails()
{
    // GET THE FILENAME FOR THE OUTPUT THUMBNAIL SHEETS
    QSettings settings;
    QString directory = settings.value("LAUCombineImagesToPDFWidget::processThumbnails", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString outputString = QFileDialog::getSaveFileName(this, QString("Save print sheets..."), directory, QString("*.tif"));
    if (outputString.isEmpty()) {
        return (false);
    }
    settings.setValue("LAUCombineImagesToPDFWidget::processThumbnails", outputString);

    // CHOP OFF THE FILE EXTENSION, IF IT EXISTS
    if (outputString.toLower().endsWith(".tif")) {
        outputString.chop(4);
    } else if (outputString.toLower().endsWith(".tiff")) {
        outputString.chop(5);
    }

    // GET THE FILENAME FOR THE TRANSFORMED THUMBNAIL IMAGES
    directory = settings.value("LAUCombineImagesToPDFWidget::saveThumbnails", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString thumbnailString = QFileDialog::getSaveFileName(this, QString("Save thumbnail images..."), directory, QString("*.tif"));
    if (thumbnailString.isEmpty() == false) {
        // SAVE THE DIRECTORY TO SETTINGS FOR NEXT TIME
        settings.setValue("LAUCombineImagesToPDFWidget::saveThumbnails", thumbnailString);

        // CHOP OFF THE FILE EXTENSION, IF IT EXISTS
        if (thumbnailString.toLower().endsWith(".tif")) {
            thumbnailString.chop(4);
        } else if (thumbnailString.toLower().endsWith(".tiff")) {
            thumbnailString.chop(5);
        }
    }

    // GET THE FILENAME FOR THE NOISEY THUMBNAIL IMAGES
    QString noiseyString;
    if (simulateNoiseCheckBox->currentText() == QString("YES")) {
        directory = settings.value("LAUCombineImagesToPDFWidget::noiseyThumbnailImages", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        noiseyString = QFileDialog::getSaveFileName(this, QString("Save noisey thumbnail images..."), directory, QString("*.tif"));
        if (noiseyString.isEmpty() == false) {
            // SAVE THE DIRECTORY TO SETTINGS FOR NEXT TIME
            settings.setValue("LAUCombineImagesToPDFWidget::noiseyThumbnailImages", noiseyString);

            // CHOP OFF THE FILE EXTENSION, IF IT EXISTS
            if (noiseyString.toLower().endsWith(".tif")) {
                noiseyString.chop(4);
            } else if (noiseyString.toLower().endsWith(".tiff")) {
                noiseyString.chop(5);
            }
        }
    }

    // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
    QStringList filters;
    filters << "*.tif";
    QStringList strings = QDir(directoryString).entryList(filters, QDir::Files);

    //while (strings.count() > 1000) {
    //    strings.removeLast();
    //}

    int cols = imageColsSpinBox->value();
    int rows = imageRowsSpinBox->value();
    double resolution = outputResolutionSpinBox->value();
    double pageWidth = pageWidthSpinBox->value();
    double pageHeight = pageHeightSpinBox->value();
    double leftMargin = leftMarginSpinBox->value();
    double topMargin = topMarginSpinBox->value();
    double gap = spaceBetweenImages->value();

    LAUMemoryObject object(pageWidth * resolution, pageHeight * resolution, 3, sizeof(unsigned char));
    object.setResolution(static_cast<float>(resolution));

    QImage image(object.constPointer(), object.width(), object.height(), object.step(), QImage::Format_RGB888);
    image.fill(Qt::white);

    // DRAW THE EDGES OF THE IMAGE THUMBNAILS
    QPainter painter(&image);
    painter.setBrush(QBrush(QColor(128, 128, 255), Qt::SolidPattern));
    painter.setPen(QPen(QColor(0, 0, 0), 0, Qt::SolidLine));

    int counter = 0;
    int index = 0;

    QProgressDialog progressDialog(QString("Generating thumbnail images..."), QString("Abort"), 0, strings.count() - 1, this, Qt::Sheet);
    progressDialog.setModal(true);
    progressDialog.show();
    while (strings.isEmpty() == false) {
        double squareWidth = (pageWidth - 2.0 * leftMargin - gap * (cols - 1)) / cols;
        double squareHeight = (pageHeight - 2.0 * topMargin - gap * (rows - 1)) / rows;
        for (int row = 0; row < rows; row++) {
            double squareTopEdge = topMargin + (squareHeight + gap) * row;
            for (int col = 0; col < cols; col++) {
                if (strings.count() > 0) {
                    progressDialog.setValue(counter);
                    if (progressDialog.wasCanceled()) {
                        break;
                    }

                    QImage image(QString("%1/%2").arg(directoryString).arg(strings.takeFirst()));
                    if (image.isNull() == false) {
                        if ((pageWidth > pageHeight) && (image.width() < image.height())) {
                            image = image.transformed(QTransform().rotate(90.0));
                        } else if ((pageWidth < pageHeight) && (image.width() > image.height())) {
                            image = image.transformed(QTransform().rotate(90.0));
                        }

                        // GET THE RECTANGLE FOR THE IMAGE IN THE FINAL OUTPUT
                        double squareLeftEdge = leftMargin + (squareWidth + gap) * col;
                        QRect rect(squareLeftEdge * resolution, squareTopEdge * resolution, squareWidth * resolution, squareHeight * resolution);

                        if (thumbnailString.isEmpty() == false) {
                            // RESIZE THE IMAGE TO THE TARGET
                            image = image.scaled(rect.size());

                            // SAVE THE THUMBNAIL TO DISK
                            QString filestring = thumbnailString;
                            if (counter < 10) {
                                filestring.append(QString("0000"));
                            } else if (counter < 100) {
                                filestring.append(QString("000"));
                            } else if (counter < 1000) {
                                filestring.append(QString("00"));
                            } else if (counter < 10000) {
                                filestring.append(QString("0"));
                            }
                            filestring.append(QString("%1").arg(counter));
                            filestring.append(QString(".tif"));
                            image.save(filestring, "TIFF");
                        }

                        if (simulateNoiseCheckBox->currentText() == QString("YES")) {
                            LAUMemoryObject object = simulateNoise(image, rect, resolution);

                            // SAVE THE THUMBNAIL TO DISK
                            QString filestring = noiseyString;
                            if (counter < 10) {
                                filestring.append(QString("0000"));
                            } else if (counter < 100) {
                                filestring.append(QString("000"));
                            } else if (counter < 1000) {
                                filestring.append(QString("00"));
                            } else if (counter < 10000) {
                                filestring.append(QString("0"));
                            }
                            filestring.append(QString("%1").arg(counter));
                            filestring.append(QString(".tif"));
                            object.save(filestring);
                        }

                        painter.drawImage(rect, image, image.rect());
                    }
                    counter++;
                }
            }
        }

        QString filestring = outputString;
        if (index < 10) {
            filestring.append(QString("0000"));
        } else if (index < 100) {
            filestring.append(QString("000"));
        } else if (index < 1000) {
            filestring.append(QString("00"));
        } else if (index < 10000) {
            filestring.append(QString("0"));
        }
        filestring.append(QString("%1").arg(index));

#define OUTPUTPDFFILES
#ifdef OUTPUTPDFFILES
        // SAVE THE MEMORY OBJECT TO A TIFF FILE IN THE TEMPORARY DIRECTORY
        QString tempString = QDir::tempPath().append("/temp.tif");
        if (object.save(tempString)) {
            filestring.append(QString(".pdf"));
            LAUTiff2PdfObject pdfObject(tempString, filestring);
        }
#else
        filestring.append(QString(".tif"));
        object.save(filestring);
#endif
        index++;
    }
    painter.end();

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUCombineImagesToPDFDialog::onLoadImages()
{
    QSettings settings;
    QString directory = settings.value("LAUCombineImagesToPDFDialog::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    directory = QFileDialog::getExistingDirectory(this, QString("Load source directory..."), directory);
    if (directory.isEmpty() == false) {
        settings.setValue("LAUCombineImagesToPDFDialog::lastUsedDirectory", directory);
        widget->setDirectoryString(directory);

        // ENABLE THE OK BUTTON NOW THAT WE HAVE A VALID SOURCE IMAGE DIRECTORY
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUCombineImagesToPDFWidget::simulateNoise(QImage image, QRect position, float resolution)
{
    // SEE IF WE HAVE A VALID NOISE STRING
    if (noiseObject.isNull()) {
        noiseObject = LAUMemoryObject((QString()));
    }

    // SEE IF WE NEED TO RESAMPLE THE THUMBNAIL TO THE NOISE OBJECT RESOLUTION
    if (qAbs(resolution - noiseObject.resolution()) > 0.001f) {
        int newLft = qRound((double)position.left() / (double)resolution * (double)noiseObject.resolution());
        int newWdt = qRound((double)position.width() / (double)resolution * (double)noiseObject.resolution());
        int newTop = qRound((double)position.top() / (double)resolution * (double)noiseObject.resolution());
        int newHgt = qRound((double)position.height() / (double)resolution * (double)noiseObject.resolution());

        position = QRect(newLft, newTop, newWdt, newHgt);
    }

    image = image.convertToFormat(QImage::Format_Grayscale8);
    image = image.scaled(position.width(), position.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    LAUMemoryObject object((unsigned int)image.width(), (unsigned int)image.height(), 1, sizeof(unsigned short));
    for (unsigned int row = 0; row < object.height(); row++) {
        unsigned short *buffer = reinterpret_cast<unsigned short *>(object.scanLine(row));
        for (unsigned int col = 0; col < object.width(); col++) {
            double pixel = (double)qRed(image.pixel((int)col, (int)row)) / 255.0;;
            unsigned int noiseCol = col + (unsigned int)position.left();
            if (noiseCol < noiseObject.width()) {
                unsigned int noiseRow = (unsigned int)qRound((double)(noiseObject.height() - 1) * pixel);
                buffer[col] = reinterpret_cast<unsigned short *>(noiseObject.constScanLine(noiseRow))[noiseCol];
            }
        }
    }
    return (object);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUCombineImagesToPDFDialog::preProcessDataSet()
{
    // LET THE USER SELECT THE INPUT IMAGE DIRECTORY
    QSettings settings;
    QString sourceDirectory = settings.value("LAUCombineImagesToPDFDialog::preProcessDataSet", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    sourceDirectory = QFileDialog::getExistingDirectory(nullptr, QString("Load preprocess directory..."), sourceDirectory);
    if (sourceDirectory.isEmpty() == false) {
        settings.setValue("LAUCombineImagesToPDFDialog::preProcessDataSet", sourceDirectory);
    }

    // NOW LET THE USER SPECIFY THE OUTPUT FILES
    QString sinkDirectory = settings.value("LAUCombineImagesToPDFWidget::postProcessDataSet", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString string = QFileDialog::getSaveFileName(nullptr, QString("Save thumbnails..."), sinkDirectory, QString("*.tif"));
    if (string.isEmpty()) {
        return;
    }
    settings.setValue("LAUCombineImagesToPDFWidget::postProcessDataSet", string);

    // CHOP OFF THE FILE EXTENSION, IF IT EXISTS
    if (string.toLower().endsWith(".tif")) {
        string.chop(4);
    } else if (string.toLower().endsWith(".tiff")) {
        string.chop(5);
    }

    // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
    QStringList filters;
    filters << "*.jpg";
    QStringList strings = QDir(sourceDirectory).entryList(filters, QDir::Files);

    QProgressDialog progressDialog(QString("Pre-processing images..."), QString("Abort"), 0, strings.count() - 1, nullptr);
    progressDialog.setModal(true);
    progressDialog.show();

    int counter = 0;
    while (strings.isEmpty() == false) {
        progressDialog.setValue(counter);
        if (progressDialog.wasCanceled()) {
            break;
        }

        QImage image(QString("%1/%2").arg(sourceDirectory).arg(strings.takeFirst()));
        if (image.isNull() == false) {
            QString filestring = string;
            if (counter < 10) {
                filestring.append(QString("0000"));
            } else if (counter < 100) {
                filestring.append(QString("000"));
            } else if (counter < 1000) {
                filestring.append(QString("00"));
            } else if (counter < 10000) {
                filestring.append(QString("0"));
            }
            filestring.append(QString("%1").arg(counter));
            filestring.append(QString(".tif"));
            image.save(filestring, "TIFF");
        }
        counter++;
    }
}
