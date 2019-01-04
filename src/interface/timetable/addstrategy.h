#ifndef ADDSTRATEGY_H
#define ADDSTRATEGY_H

#include <QDialog>

#include "strategies.h"

#include <deque>

namespace Ui {
class AddStrategy;
}

class AddStrategyForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddStrategyForm(QWidget *parent = nullptr);
    ~AddStrategyForm();

public slots:
    void fromEmptySolutionSelected(FromEmptySolutionPhase* fromEmptySolutionPhase);

    void localSearchSelected(FromInicialSolutionPhase* fromInicialSolutionPhase);

    void phasesChanged();

signals:
    void newStrategyAdded(GenerationStrategy* gs);

private slots:
    void on_changeInicialPhasePushButton_clicked();

    void on_newPhasePushButton_clicked();

    void on_savePushButton_clicked();

    void on_closePushButton_clicked();

    void on_deletePhasePushButton_clicked();

private:
    Ui::AddStrategy *ui;

    FromEmptySolutionPhase* fromEmptySolutionPhase = nullptr;

    std::deque<GenerationPhase*> phases;
};

#endif // ADDSTRATEGY_H
