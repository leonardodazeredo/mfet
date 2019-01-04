#include "timetableshowsolutionsform.h"
#include "ui_timetableshowsolutionsform.h"

#include "timetableviewstudentsform.h"
#include "timetableviewteachersform.h"
#include "timetableshowconflictsform.h"
#include "timetableprintform.h"

#include "timetablegenerateform.h"

#include <QMessageBox>

#include "interfaceutils.h"

#include "tcontext.h"

TimetableShowSolutionsForm::TimetableShowSolutionsForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimetableShowSolutionsForm)
{
    ui->setupUi(this);

    if (!TContext::get()->instance.internalStructureComputed) {
        TContext::get()->instance.computeInternalStructure(this);
    }

    connect(ui->solutionsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_shortcutShowSoftConflictsPushButton_clicked()));

    solutionsChanged();
}

TimetableShowSolutionsForm::~TimetableShowSolutionsForm()
{
    delete ui;
}

void TimetableShowSolutionsForm::solutionsChanged()
{
    QString s;
    ui->solutionsListWidget->clear();

    int k=0;
    for(size_t i=0; i<TContext::get()->solutions.size(); i++){
        Solution solution = *std::next(TContext::get()->solutions.begin(), i);

        s=solution.getCostDetailsString(Enums::Short);

        ui->solutionsListWidget->addItem(s);

        k++;
        if(defs::USE_GUI_COLORS)
            ui->solutionsListWidget->item(k-1)->setBackground(ui->solutionsListWidget->palette().alternateBase());
    }

    if(ui->solutionsListWidget->count()>0)
        ui->solutionsListWidget->setCurrentRow(0);
}

void TimetableShowSolutionsForm::on_deletePhasePushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        return;
    }

    auto a = [&index, this](){
        TContext::get()->solutions.erase(std::next(TContext::get()->solutions.begin(), index));
        this->solutionsChanged();
    };

    InterfaceUtils::showConfirmationBox(this, a, "Confirm?");
}

void TimetableShowSolutionsForm::on_shortcutShowSoftConflictsPushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
        return;
    }

    Solution s = *std::next(TContext::get()->solutions.begin(), index);

    InterfaceUtils::openFormWindowSimple(new TimetableShowConflictsForm(this, s));
}

void TimetableShowSolutionsForm::on_shortcutViewStudentsPushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
        return;
    }

    Solution s = *std::next(TContext::get()->solutions.begin(), index);

    TimetableViewStudentsForm *form=new TimetableViewStudentsForm(this, s);

    InterfaceUtils::openFormWindowSimple(form);

    form->resizeRowsAfterShow();
}

void TimetableShowSolutionsForm::on_shortcutViewTeachersPushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
        return;
    }

    Solution s = *std::next(TContext::get()->solutions.begin(), index);

//    if(Timetable::getInstance()->instance.nInternalTeachers!=Timetable::getInstance()->instance.teachersList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some teachers. Please regenerate the timetable and then view it"));
//		return;
//	}

    TimetableViewTeachersForm *form=new TimetableViewTeachersForm(this, s);

    InterfaceUtils::openFormWindowSimple(form);

    form->resizeRowsAfterShow();
}

void TimetableShowSolutionsForm::on_shortcutsTimetablePrintPushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
        return;
    }

    Solution s = *std::next(TContext::get()->solutions.begin(), index);

//    if(Timetable::getInstance()->instance.nInternalTeachers!=Timetable::getInstance()->instance.teachersList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some teachers. Please regenerate the timetable and then view it"));
//		return;
//	}

    StartTimetablePrint::startTimetablePrint(this);
}

void TimetableShowSolutionsForm::on_closePushButton_clicked()
{
    close();
}

void TimetableShowSolutionsForm::on_savePushButton_clicked()
{
    TContext::get()->saveSolutions(this);
}

void TimetableShowSolutionsForm::on_loadPushButton_clicked()
{
    TContext::get()->loadSolutions(this);
    solutionsChanged();
}

void TimetableShowSolutionsForm::on_addPushButton_clicked()
{
    Solution s(&TContext::get()->instance);

    TimetableViewStudentsForm *form=new TimetableViewStudentsForm(this, s);

    InterfaceUtils::openFormWindowSimple(form);

    form->resizeRowsAfterShow();
}

void TimetableShowSolutionsForm::on_runPhasePushButton_clicked()
{
    int index = ui->solutionsListWidget->currentRow();

    if (index < 0) {
        QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
        return;
    }

    TimetableGenerateForm *form=new TimetableGenerateForm(this);
    form->startImproving(index);
    InterfaceUtils::openFormWindowModal(form);
    solutionsChanged();
}
