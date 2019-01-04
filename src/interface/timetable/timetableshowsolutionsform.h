#ifndef TIMETABLESHOWSOLUTIONSFORM_H
#define TIMETABLESHOWSOLUTIONSFORM_H

#include <QDialog>

namespace Ui {
class TimetableShowSolutionsForm;
}

class TimetableShowSolutionsForm : public QDialog
{
    Q_OBJECT

public:
    explicit TimetableShowSolutionsForm(QWidget *parent = 0);
    ~TimetableShowSolutionsForm();


public slots:
    void solutionsChanged();

private slots:
    void on_deletePhasePushButton_clicked();

    void on_shortcutShowSoftConflictsPushButton_clicked();

    void on_shortcutViewStudentsPushButton_clicked();

    void on_shortcutViewTeachersPushButton_clicked();

    void on_shortcutsTimetablePrintPushButton_clicked();

    void on_closePushButton_clicked();

    void on_savePushButton_clicked();

    void on_loadPushButton_clicked();

    void on_addPushButton_clicked();

    void on_runPhasePushButton_clicked();

private:
    Ui::TimetableShowSolutionsForm *ui;
};

#endif // TIMETABLESHOWSOLUTIONSFORM_H
