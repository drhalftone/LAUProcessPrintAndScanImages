#ifndef LAUDEFAULTDIRECTORIESWIDGET_H
#define LAUDEFAULTDIRECTORIESWIDGET_H

#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUDefaultDirectoriesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUDefaultDirectoriesWidget(QStringList kys, QStringList tgs = QStringList(), QWidget *parent = nullptr);

    QStringList directories() const
    {
        return (tags);
    }

public slots:
    void onButtonClicked();

protected:
    void showEvent(QShowEvent *event)
    {
        int labelWidth = 0;
        for (int n = 0; n < labels.count(); n++) {
            labelWidth = qMax(labelWidth, labels.at(n)->width());
        }
        for (int n = 0; n < labels.count(); n++) {
            labels.at(n)->setFixedWidth(labelWidth);
        }
    }

private:
    QStringList keys;
    QStringList tags;

    QList<QLabel *> labels;
    QList<QLineEdit *> lineEdits;
    QList<QPushButton *> pushButtons;

signals:
    void emitDirectoryChanged(QString, QString);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUDefaultDirectoriesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUDefaultDirectoriesDialog(QWidget *parent = nullptr);

    static QString sourceImageDirectory;
    static QString prestineSheetsDirectory;
    static QString prestineThumbnailDirectory;
    static QString printedSheetsDirectory;
    static QString printedThumbnailsDirectory;
    static QString warpedThumbnailsDirectory;

    static void load();
    static void save();
    static bool isValid();

protected:
    void accept();

private:
    LAUDefaultDirectoriesWidget *widget;
};

#endif // LAUDEFAULTDIRECTORIESWIDGET_H
