#include "laufindgridbatchwidget.h"
#include "laudefaultdirectorieswidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFindGridBatchDialog::LAUFindGridBatchDialog(QStringList strings, QWidget *parent) : QDialog(parent), counter(0), abortFlag(false), filenames(strings), progress(nullptr)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->setWindowTitle(QString("Thumbnail Layout Dialog"));

    // SET THE SINK FILE NAME BASED ON THE DEFAULT DIRECTORIES
    sinkName = QString("%1/printedThumbnail").arg(LAUDefaultDirectoriesDialog::printedThumbnailsDirectory);

    // CREATE A GLWIDGET FOR DISPLAY AND PROCESSING OF MEMORY OBJECTS
    widget = new LAUFindGridGLWidget(LAUMemoryObject());
    widget->setMinimumSize(640, 480);
    connect(this, SIGNAL(emitObject(LAUMemoryObject)), widget, SLOT(onUpdateObject(LAUMemoryObject)), Qt::QueuedConnection);
    connect(widget, SIGNAL(emitObject(LAUMemoryObject)), this, SLOT(onUpdateObject(LAUMemoryObject)), Qt::QueuedConnection);
    this->layout()->addWidget(widget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(onAbort()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onUpdateObject()));
    this->layout()->addWidget(buttonBox);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridBatchDialog::showEvent(QShowEvent *)
{
    // CREATE A PROGRESS BAR
    progress = new QProgressDialog(QString("Processing printed sheets..."), QString("Abort"), 0, filenames.count(), this, Qt::Sheet);
    progress->show();

    // START THE PROCESSING OF OBJECTS
    emit emitObject(LAUMemoryObject());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridBatchDialog::onUpdateObject(LAUMemoryObject obj)
{
    if (progress->wasCanceled()) {
        QDialog::reject();
    } else {
        // EXTRACT ALL THE IMAGES OF THE JUST PROCESSED MEMORY OBJECT
        if (obj.isValid()) {
            for (unsigned int row = 0; row < widget->rows(); row++) {
                for (unsigned int col = 0; col < widget->cols(); col++) {
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
        }

        // LOAD THE NEXT IMAGE IN THE INPUT LIST
        if (filenames.count() > 0) {
            QString string = filenames.first().split("/").last();
            string.chop(string.length() - string.lastIndexOf('.'));
            counter = 100 * string.right(5).toInt();

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
