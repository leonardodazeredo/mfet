#ifndef GENERAL_H
#define GENERAL_H

#include <QPair>
#include <QHash>

#include "enums.h"

class Activity;
class Teacher;

typedef int ActId;
typedef int StuSetId;
typedef int TchId;
typedef int TimeSlotId;

typedef int ActIdex;
typedef int StuSetIdex;
typedef int TchIdex;

typedef QPair<Activity*,Teacher*> CandidateActTch;
typedef QPair<TimeSlotId,Teacher*> CandidateTstTch;

struct Move {
      virtual ~Move();
};

struct MoveN1andN2 : Move {
    int tchi;
    int ts1;
    int ts2;
};

struct MoveN3 : Move {
    int ai1;
    int ai2;
};

struct MoveN4 : Move {
    int ai;
    int newTchi;
};

struct MoveN5 : Move {
    int ai1;
    int ai2;
};

class ConstraintConflictDataForGroup{
public:
    ConstraintConflictDataForGroup();

    ConstraintConflictDataForGroup(Enums::ConstraintGroup g);

    Enums::ConstraintGroup group;

    QList<double> conflictsCostList;
    QList<QString> conflictsDescriptionList;
    double costTotal;

    void computeData();

    void clear();
};

class ConstraintConflictData{
public:
    ConstraintConflictData();

    QHash<Enums::ConstraintGroup, ConstraintConflictDataForGroup> group_data_map;

    QList<double> conflictsCostList();
    QList<double> conflictsCostList(Enums::ConstraintGroup g);

    QList<QString> conflictsDescriptionList();
    QList<QString> conflictsDescriptionList(Enums::ConstraintGroup g);

    double costTotal();
    double costTotal(Enums::ConstraintGroup g);

    void addConflict(Enums::ConstraintGroup g, QString desc, double cost);

    void computeData();

    void clear();
};

class SubjectPreferenceDataForLevel{
public:
    SubjectPreferenceDataForLevel();

    SubjectPreferenceDataForLevel(Enums::SubjectPreference l);

    Enums::SubjectPreference level;

    QHash<int, int> teachersIndexes_count_map;

    void computeData();

    void clear();
};

class SubjectPreferenceData{
public:
    SubjectPreferenceData();

    QHash<Enums::SubjectPreference, SubjectPreferenceDataForLevel> level_data_map;

    QList<int> teachersIndexesList();
    QList<int> teachersIndexesList(Enums::SubjectPreference l);

    void addPreferenceData(Enums::SubjectPreference l, int teacherIndex);

    void computeData();

    void clear();
};

#endif // GENERAL_H
