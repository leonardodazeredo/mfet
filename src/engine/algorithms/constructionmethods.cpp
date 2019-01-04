#include "constructionmethods.h"

#include <QtMath>
#include <QVector>

#include "strategies.h"
#include "generate.h"
#include "instance.h"
#include "solution.h"
#include "tcontext.h"

#include "consoleutils.h"
#include "randutils.h"
#include "functionaltutils.h"
#include "decimalutils.h"
#include "listutils.h"
#include "stringutils.h"

using namespace utils;

Solution ConstructionBySubGroups::construct(float alpha)
{
    assert(instance.internalStructureComputed);
    assert(instance.initialized);
    assert(alpha >= 0.0f && alpha <= 1.0f);
    this->alpha = alpha;

    Solution solucao(&instance);

    emit(constructionStarted(this));

    while (!construct(solucao,alpha)) {
        solucao.makeUnallocated();

        if (getGenerationStrategy()!=nullptr) {
            getGenerationStrategy()->n_reinicios++;
        }

        emit(constructionReStarted(this));
    }

    emit(constructionFinished(solucao.getCostDetailsString(), this));

    return solucao;
}

Solution ConstructionByActivities::construct(float alpha)
{
    assert(instance.internalStructureComputed);
    assert(instance.initialized);
    assert(alpha >= 0.0f && alpha <= 1.0f);
    this->alpha = alpha;

    Solution solucao(&instance);

    emit(constructionStarted(this));

    while (!construct(solucao,alpha)) {
        solucao.makeUnallocated();

        if (getGenerationStrategy()!=nullptr) {
            getGenerationStrategy()->n_reinicios++;
        }

        emit(constructionReStarted(this));
    }

    emit(constructionFinished(solucao.getCostDetailsString(), this));

    return solucao;
}

bool ConstructionBySubGroups::construct(Solution &solucao, float alpha)
{
    QHash<StudentsSubgroup*,QList<int>> studentSet_UnallocatedActsList_Map;

    QHash<StudentsSubgroup*,QVector<bool>> studentSet_slotsOcupados_Map;

    QHash<Teacher*,QVector<bool>> teacher_slotsOcupados_Map;

    for (auto studentSet : instance.directSubgroupsList){
        QVector<bool> sTslotsOcupados(instance.getNTimeSlots());
        for (auto natl : studentSet->notAvailableTimeSlots)
            sTslotsOcupados[natl] = true;

        studentSet_slotsOcupados_Map.insert(studentSet, sTslotsOcupados);

        QList<int> actForSubgroup(studentSet->activitiesForSubgroup);
        studentSet_UnallocatedActsList_Map.insert(studentSet, actForSubgroup);
    }

    for (auto teacher : instance.activeTeachersList){
        QVector<bool> tcHslotsOcupados(instance.getNTimeSlots());
        for (auto natl : teacher->notAvailableTimeSlots)
            tcHslotsOcupados[natl] = true;

        teacher_slotsOcupados_Map.insert(teacher, tcHslotsOcupados);
    }

    QList<StudentsSubgroup*> studentSetList(instance.directSubgroupsList);
    random::shuffle(studentSetList);

    QVector<TimeSlotId> timeSlots(instance.getNTimeSlots());
    std::iota(timeSlots.begin(), timeSlots.end(), 0);
    random::shuffle(timeSlots);

    for (auto studentSet : studentSetList){

        if (studentSet_UnallocatedActsList_Map[studentSet].isEmpty())
            continue;

        for (TimeSlotId tslot : timeSlots) {

            if (studentSet_UnallocatedActsList_Map[studentSet].isEmpty())
                break;

            if (studentSet_slotsOcupados_Map[studentSet][tslot])
                continue;

            QVector<CandidateActTch> candidatos;

            for (auto i : studentSet_UnallocatedActsList_Map[studentSet]) {
                Activity* act = instance.activeActivitiesList[i];

                auto pred = [&tslot,&studentSet_slotsOcupados_Map,this](int sti) {return studentSet_slotsOcupados_Map[this->instance.directSubgroupsList[sti]][tslot];};
                auto auxl = functional::filter(act->iSubgroupsList.toStdList(), pred);
                if (auxl.size()!=0)
                    continue;

                QVector<Teacher*> teachersPossiveis;

                for (auto teacher: instance.subjectsList[act->subjectIndex]->preferredTeachers) {
                    if (teacher_slotsOcupados_Map[teacher][tslot])
                        continue;

                    teachersPossiveis.push_back(teacher);
                }

                if (!teachersPossiveis.size()) {
                    for (auto teacher: instance.subjectsList[act->subjectIndex]->indifferentTeachers) {
                        if (teacher_slotsOcupados_Map[teacher][tslot])
                            continue;

                        teachersPossiveis.push_back(teacher);
                    }
                }

                if (!teachersPossiveis.size()) {
                    for (auto teacher: instance.activeTeachersList) {
                        if (teacher_slotsOcupados_Map[teacher][tslot])
                            continue;

                        CandidateActTch candidato(act, teacher);
                        candidatos.push_back(candidato);
                    }
                }
                else{
                    for (auto teacher: teachersPossiveis) {
                        if (teacher_slotsOcupados_Map[teacher][tslot])
                            continue;

                        CandidateActTch candidato(act, teacher);
                        candidatos.push_back(candidato);
                    }
                }
            }

            if (!candidatos.size()) {
                return false;
            }

            Solution s;
            vector<QPair<Solution,CandidateActTch>> solucoesParciais;

            for (auto candidato : candidatos){
                s = solucao;
                s.adicionarCandidato(tslot, candidato);
                s.cost();
                QPair<Solution,CandidateActTch> p(s, candidato);
                solucoesParciais.push_back(p);
            }

            assert(solucoesParciais.size() > 0);

            auto comp = [] (QPair<Solution,CandidateActTch> &a, QPair<Solution,CandidateActTch> &b) {return a.first.cost() < b.first.cost();};
            std::sort(solucoesParciais.begin(),solucoesParciais.end(), comp);

            double limite = solucoesParciais[solucoesParciais.size()-1].first.cost() - DOUBLE(alpha)*(solucoesParciais[solucoesParciais.size()-1].first.cost() - solucoesParciais[0].first.cost());
            limite = decimal::round_up(limite, 6);

            auto pred = [&limite](QPair<Solution,CandidateActTch> p) {return p.first.cost() <= limite;};
            vector<QPair<Solution,CandidateActTch>> RCL = functional::filter(solucoesParciais, pred);

            assert(RCL.size() > 0);

            QPair<Solution,CandidateActTch> aux = random::pick(RCL);
            solucao = aux.first;

            Activity* actSelecionada = aux.second.first;

            for(auto sti : actSelecionada->iSubgroupsList){
                auto sg = instance.directSubgroupsList[sti];

                for (TimeSlotId t = tslot; t < tslot + actSelecionada->duration; ++t) {
                    studentSet_slotsOcupados_Map[sg][t] = true;
                }

                studentSet_UnallocatedActsList_Map[sg].removeAll(instance.getActivityIndex(actSelecionada->id));
            }

            for (TimeSlotId t = tslot; t < tslot + actSelecionada->duration; ++t) {
                teacher_slotsOcupados_Map[aux.second.second][t] = true;
            }
        }

        if (!studentSet_UnallocatedActsList_Map[studentSet].isEmpty()){
            return false;
        }
    }

    return true;
}

QString ConstructionBySubGroups::description() {
    QString s = shortDescription();
    return s;
}

QString ConstructionBySubGroups::shortDescription() const{
    return enums::enumIndexToStr(Enums::Construction_By_SubGroups);
}

bool ConstructionByActivities::construct(Solution &solucao, float alpha)
{
    QHash<StudentsSubgroup*,QVector<bool>> studentSet_slotsOcupados_Map;

    QHash<Teacher*,QVector<bool>> teacher_slotsOcupados_Map;

    for (auto studentSet : instance.directSubgroupsList){
        QVector<bool> sTslotsOcupados(instance.getNTimeSlots());
        for (auto natl : studentSet->notAvailableTimeSlots)
            sTslotsOcupados[natl] = true;

        studentSet_slotsOcupados_Map.insert(studentSet, sTslotsOcupados);
    }

    for (auto teacher : instance.activeTeachersList){
        QVector<bool> tcHslotsOcupados(instance.getNTimeSlots());
        for (auto natl : teacher->notAvailableTimeSlots)
            tcHslotsOcupados[natl] = true;

        teacher_slotsOcupados_Map.insert(teacher, tcHslotsOcupados);
    }

    ActivitiesList activitiesList(instance.activeActivitiesList);
    random::shuffle(activitiesList);

    QVector<TimeSlotId> timeSlots(instance.getNTimeSlots());
    std::iota(timeSlots.begin(), timeSlots.end(), 0);
    random::shuffle(timeSlots);

    for (Activity* act: activitiesList){
        QVector<CandidateTstTch> candidatos;

        QVector<bool> ocupacoesDeSlots(studentSet_slotsOcupados_Map.value(instance.directSubgroupsList.at(act->iSubgroupsList.at(0))));

        for (int sti = 1; sti < act->iSubgroupsList.size(); ++sti) {
            auto st = instance.directSubgroupsList.at(sti);
            ocupacoesDeSlots = functional::zip(ocupacoesDeSlots, studentSet_slotsOcupados_Map.value(st), [](bool a, bool b){return a and b;});
        }

        for (TimeSlotId tslot : timeSlots) {

            if (ocupacoesDeSlots[tslot]) {
                continue;
            }

            vector<Teacher*> teachersPossiveis;

            for (auto teacher: instance.subjectsList[act->subjectIndex]->preferredTeachers) {
                if (teacher_slotsOcupados_Map[teacher][tslot])
                    continue;

                teachersPossiveis.push_back(teacher);
            }

            if (!teachersPossiveis.size()) {
                for (auto teacher: instance.subjectsList[act->subjectIndex]->indifferentTeachers) {
                    if (teacher_slotsOcupados_Map[teacher][tslot])
                        continue;

                    teachersPossiveis.push_back(teacher);
                }
            }

            if (!teachersPossiveis.size()) {
                for (auto teacher: instance.activeTeachersList) {
                    if (teacher_slotsOcupados_Map[teacher][tslot])
                        continue;

                    CandidateTstTch candidato(tslot, teacher);
                    candidatos.push_back(candidato);
                }
            }
            else{
                for (auto teacher: teachersPossiveis) {
                    if (teacher_slotsOcupados_Map[teacher][tslot])
                        continue;

                    CandidateTstTch candidato(tslot, teacher);
                    candidatos.push_back(candidato);
                }
            }
        }

        if (!candidatos.size()) {
            return false;
        }

        Solution s;
        vector<QPair<Solution,CandidateTstTch>> solucoesParciais;

        for (auto candidato : candidatos){
            s = solucao;
            s.adicionarCandidato(act, candidato);
            s.cost();
            QPair<Solution,CandidateTstTch> p(s, candidato);
            solucoesParciais.push_back(p);
        }

        assert(solucoesParciais.size() > 0);

        auto comp = [] (QPair<Solution,CandidateTstTch> &a, QPair<Solution,CandidateTstTch> &b) {return a.first.cost() < b.first.cost();};
        std::sort(solucoesParciais.begin(),solucoesParciais.end(), comp);

        double limite = solucoesParciais[solucoesParciais.size()-1].first.cost() - DOUBLE(alpha)*(solucoesParciais[solucoesParciais.size()-1].first.cost() - solucoesParciais[0].first.cost());
        limite = decimal::round_up(limite, 6);

        auto pred = [&limite](QPair<Solution,CandidateTstTch> p) {return p.first.cost() <= limite;};
        vector<QPair<Solution,CandidateTstTch>> RCL = functional::filter(solucoesParciais, pred);

        assert(RCL.size() > 0);

        QPair<Solution,CandidateTstTch> aux = random::pick(RCL);
        solucao = aux.first;

        for(auto sti : act->iSubgroupsList){
            auto sg = instance.directSubgroupsList[sti];

            for (TimeSlotId t = aux.second.first; t < aux.second.first + act->duration; ++t) {
                studentSet_slotsOcupados_Map[sg][t] = true;
            }
        }

        for (TimeSlotId t = aux.second.first; t < aux.second.first + act->duration; ++t) {
            teacher_slotsOcupados_Map[aux.second.second][t] = true;
        }
    }

    return true;
}

QString ConstructionByActivities::description() {
    QString s = shortDescription();
    return s;
}

QString ConstructionByActivities::shortDescription() const{
    return enums::enumIndexToStr(Enums::Construction_By_Activities);
}


GenerationPhase *ConstructionMethod::getParent() const
{
    return parent;
}

void ConstructionMethod::setParent(GenerationPhase *value)
{
    parent = value;
}

int ConstructionMethod::level() const
{
    return parent!=nullptr ? parent->level() + 1 : 0;
}

GenerationStrategy *ConstructionMethod::getGenerationStrategy()
{
    return generationStrategy;
}

void ConstructionMethod::setGenerationStrategy(GenerationStrategy *value)
{
    generationStrategy = value;
}

void ConstructionMethod::logConstructionStarted(ConstructionMethod *constructionMethod)
{
    console::coutnl("%1Construção - %2 - Reinicios:",
                C_STR(strings::makeIndent(level())),
                C_STR(strings::number(constructionMethod->alpha)));
}

void ConstructionMethod::logConstructionReStarted(ConstructionMethod *constructionMethod)
{
    Q_UNUSED(constructionMethod)
    console::coutnl(" ¬");
}

void ConstructionMethod::logConstructionFinished(QString s, ConstructionMethod *constructionMethod)
{
    Q_UNUSED(constructionMethod)
    console::coutnl("\n%1Solução construida: %2",
                C_STR(strings::makeIndent(level())),
                C_STR(s));
}
