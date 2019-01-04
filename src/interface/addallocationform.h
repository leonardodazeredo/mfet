#ifndef ADDALLOCATIONFORM_H
#define ADDALLOCATIONFORM_H

#include <QDialog>

class Solution;

namespace Ui {
class AddAllocationForm;
}

class AddAllocationForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddAllocationForm(QWidget *parent, Solution &solution);
    ~AddAllocationForm();

signals:

    void allocationAdded();

private slots:
    void on_addPushButton_clicked();

    void on_closePushButton_clicked();

    void on_helpPushButton_clicked();

    void updateActList();

    void updateTchList();

    void updateTimeSlotList();

    void updateLists();

private:
    Ui::AddAllocationForm *ui;

    Solution &solution;

    QHash<int, int> listItemToActId_map;
};

#endif // ADDALLOCATIONFORM_H
