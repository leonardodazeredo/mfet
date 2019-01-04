#include "addallocationform.h"
#include "ui_addallocationform.h"

#include "centerwidgetonscreen.h"

#include "solution.h"

#include <QSettings>

#include "general.h"

#include "functionaltutils.h"

#include <QMessageBox>

#include "tcontext.h"


AddAllocationForm::AddAllocationForm(QWidget *parent, Solution &solution) :
    QDialog(parent),
    ui(new Ui::AddAllocationForm), solution(solution)
{
    ui->setupUi(this);

    updateLists();

    ui->activitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->teachersListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->timeSlotsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this, SIGNAL(allocationAdded()), parent, SLOT(updateStudentsTimetableTable()));

    centerWidgetOnScreen(this);
    restoreFETDialogGeometry(this);
}

AddAllocationForm::~AddAllocationForm()
{
    delete ui;
}

void AddAllocationForm::on_addPushButton_clicked()
{
    int ai = listItemToActId_map[ui->activitiesListWidget->currentRow()];
    int ti = ui->teachersListWidget->currentRow();
    int ts = ui->timeSlotsListWidget->currentRow();

    if(ai < 0 or ti < 0 or ts < 0){
        QMessageBox::warning(this, tr("Warning"), tr("Invalid selection."));
        return;
    }

    bool valid = true;

    auto it = solution.times.begin();
    auto itEnd = solution.times.begin() + solution.instance->activeActivitiesList.size();

    for (; it != itEnd; it++)
    {
        int tsValue = (*it);
        if (tsValue == ts) {
            int actIndx = std::distance(solution.times.begin(), it);

            QList<int> l1(solution.instance->activeActivitiesList[actIndx]->iSubgroupsList);
            QList<int> l2(solution.instance->activeActivitiesList[ai]->iSubgroupsList);

            QSet<int> intersection = l1.toSet().intersect(l2.toSet());

            if (!intersection.empty()) {
                valid = false;
            }
        }
    }

    if (!valid) {
        QMessageBox::warning(this, tr("Warning"), tr("The selected time splot is unavailable."));
        return;
    }

    Teacher* tch = TContext::get()->instance.activeTeachersList[ti];

    CandidateTstTch c(ts, tch);

    this->solution.adicionarCandidato(ai, c);

    emit(allocationAdded());

    updateLists();
}

void AddAllocationForm::on_closePushButton_clicked()
{
    close();
}

void AddAllocationForm::on_helpPushButton_clicked()
{

}

void AddAllocationForm::updateActList()
{
    int nacts=0, nsubacts=0, nh=0;
    int ninact=0, ninacth=0;

    QString s;
    ui->activitiesListWidget->clear();
    listItemToActId_map.clear();

    int k=0;
    int il=0;
    int maxL=0;
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){

        if (solution.times[i]!=UNALLOCATED_TIME) {
            continue;
        }

        Activity* act=TContext::get()->instance.activeActivitiesList[i];

        s="";

        s+=act->getDescription();

        if (s.length() > maxL) {
            maxL = s.length();
        }

        ui->activitiesListWidget->addItem(s);
        listItemToActId_map.insert(il++, i);

        k++;
        if(defs::USE_GUI_COLORS && !act->active)
            ui->activitiesListWidget->item(k-1)->setBackground(ui->activitiesListWidget->palette().alternateBase());

        if(act->id==act->activityGroupId || act->activityGroupId==0)
            nacts++;
        nsubacts++;

        nh+=act->duration;

        if(!act->active){
            ninact++;
            ninacth+=act->duration;
        }

        if(act->activityGroupId==0 || (i<TContext::get()->instance.activeActivitiesList.size()-1 && act->activityGroupId!=TContext::get()->instance.activeActivitiesList[i + 1]->activityGroupId)){
            ui->activitiesListWidget->addItem("");
            listItemToActId_map.insert(il++, -1);
        }
    }

    for (auto itemIndex: listItemToActId_map.keys()){
        if (listItemToActId_map.value(itemIndex)!=-1) {
            continue;
        }

        QString auxs;
        ui->activitiesListWidget->item(itemIndex)->setText(auxs.fill('-', maxL));
    }
}

void AddAllocationForm::updateTchList()
{
    ui->teachersListWidget->clear();
    for(int i=0; i<TContext::get()->instance.activeTeachersList.size(); i++){
        Teacher* tch=TContext::get()->instance.activeTeachersList[i];
        ui->teachersListWidget->addItem(tch->name);
    }
}

void AddAllocationForm::updateTimeSlotList()
{
    ui->timeSlotsListWidget->clear();
    for(int i=0; i<TContext::get()->instance.nDaysPerWeek; i++){
        for(int j=0; j<TContext::get()->instance.nHoursPerDay; j++){
            ui->timeSlotsListWidget->addItem(TContext::get()->instance.daysOfTheWeek[i] + " - " + TContext::get()->instance.hoursOfTheDay[j]);
        }
    }
}

void AddAllocationForm::updateLists()
{
    updateActList();
    updateTchList();
    updateTimeSlotList();
}
