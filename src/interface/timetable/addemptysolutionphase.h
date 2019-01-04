#ifndef ADDPHASE_H
#define ADDPHASE_H

#include <QDialog>

#include "ui_addemptysolutionphase.h"

class FromEmptySolutionPhase;
class FromInicialSolutionPhase;

namespace Ui {
class AddEmptySolutionPhase;
}

class AddEmptySolutionPhaseForm : public QDialog, Ui::AddEmptySolutionPhase
{
    Q_OBJECT

public:
    explicit AddEmptySolutionPhaseForm(QWidget *parent = nullptr);
    ~AddEmptySolutionPhaseForm();

signals:
    void returnFromEmptySolutionPhase(FromEmptySolutionPhase* fromEmptySolutionPhase);

private slots:
    void on_methodComboBox_currentTextChanged(const QString &arg1);

    void localSearchSelected(FromInicialSolutionPhase* fromInicialSolutionPhase);

    void on_graspLocalSearchPushButton_clicked();
    void on_graspReativoLocalSearchPushButton_clicked();
    void on_randomLocalSearchPushButton_clicked();

    void on_savePushButton_clicked();

    void on_closePushButton_clicked();

    void on_methodToolBox_currentChanged(int index);

private:
    FromInicialSolutionPhase* fromInicialSolutionPhase = nullptr;

    void init();
};

#endif // ADDPHASE_H
