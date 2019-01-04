#ifndef ADDINITIALSOLUTIONPHASE_H
#define ADDINITIALSOLUTIONPHASE_H

#include <QDialog>

class FromInicialSolutionPhase;
class FromInicialSolutionPhase;

namespace Ui {
class AddInitialSolutionPhase;
}

class AddInitialSolutionPhaseForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddInitialSolutionPhaseForm(QWidget *parent = nullptr);
    ~AddInitialSolutionPhaseForm();

signals:
    void returnFromInicialSolutionPhase(FromInicialSolutionPhase* fromInicialSolutionPhase);

private slots:
    void on_methodComboBox_currentTextChanged(const QString &arg1);

    void localSearchSelected(FromInicialSolutionPhase* fromInicialSolutionPhase);

    void on_savePushButton_clicked();

    void on_closePushButton_clicked();
    void on_methodToolBox_currentChanged(int index);

    void on_vndLocalSearchPushButton_clicked();

    void on_vnsONLocalSearchPushButton_clicked();

    void on_vnsRNLocalSearchPushButton_clicked();

private:
    Ui::AddInitialSolutionPhase *ui;

    FromInicialSolutionPhase* fromInicialSolutionPhase = nullptr;

    void init();
};

#endif // ADDINITIALSOLUTIONPHASE_H
