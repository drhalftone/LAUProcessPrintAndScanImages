#include "laufindgridbatchwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFindGridBatchDialog::LAUFindGridBatchDialog(QStringList strings, QWidget *parent) : QDialog(parent), counter(0), abortFlag(false), filenames(strings)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->setWindowTitle(QString("Thumbnail Layout Dialog"));

    QSettings settings;
    QString directory = settings.value("LAUFindGridBatchDialog::sinkImageDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    sinkName = QFileDialog::getSaveFileName(this, QString("Save thumbnails..."), directory, QString("*.tif"));
    if (sinkName.isEmpty()) {
        return;
    }
    settings.setValue("LAUFindGridBatchDialog::sinkImageDirectory", sinkName);

    // CHOP OFF THE FILE EXTENSION, IF IT EXISTS
    if (sinkName.toLower().endsWith(".tif")) {
        sinkName.chop(4);
    } else if (sinkName.toLower().endsWith(".tiff")) {
        sinkName.chop(5);
    }

    // CREATE A GLWIDGET FOR DISPLAY AND PROCESSING OF MEMORY OBJECTS
    widget = new LAUFindGridGLWidget(LAUMemoryObject());
    widget->setMinimumSize(480, 320);
    connect(this, SIGNAL(emitObject(LAUMemoryObject)), widget, SLOT(onUpdateObject(LAUMemoryObject)), Qt::QueuedConnection);
    connect(widget, SIGNAL(emitObject(LAUMemoryObject)), this, SLOT(onUpdateObject(LAUMemoryObject)), Qt::QueuedConnection);
    this->layout()->addWidget(widget);

    progress = new QProgressBar();
    this->layout()->addWidget(progress);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(onAbort()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onUpdateObject()));
    this->layout()->addWidget(buttonBox);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridBatchDialog::onUpdateObject(LAUMemoryObject obj)
{
    if (abortFlag) {
        QDialog::reject();
    } else {
        // EXTRACT ALL THE IMAGES OF THE JUST PROCESSED MEMORY OBJECT
        if (obj.isValid()) {
            for (int row = 0; row < widget->rows(); row++) {
                for (int col = 0; col < widget->cols(); col++) {
                    LAUMemoryObject object = widget->result(col, row);

                    QString filestring = sinkName;
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

                    counter++;
                }
            }
        } else {
            progress->setMaximum(filenames.count());
        }

        // LOAD THE NEXT IMAGE IN THE INPUT LIST
        while (filenames.count() > 0) {
            LAUMemoryObject obj = LAUMemoryObject(filenames.takeFirst());
            if (obj.isValid()) {
                progress->setValue(progress->maximum() - filenames.count());
                emit emitObject(LAUMemoryObject(obj));
                return;
            }
        }

        // IF WE MAKE IT THIS FAR, THEN WE MUST BE DONE
        QDialog::accept();
    }
}
