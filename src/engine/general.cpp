#include "general.h"

#include "enumutils.h"
#include "stringutils.h"
#include "functionaltutils.h"

ConstraintConflictDataForGroup::ConstraintConflictDataForGroup()
{
}

ConstraintConflictDataForGroup::ConstraintConflictDataForGroup(Enums::ConstraintGroup g) : group(g)
{
}

void ConstraintConflictDataForGroup::computeData()
{
    foreach(double cn, conflictsCostList){
        costTotal+=cn;
    }

    assert(conflictsCostList.count()==conflictsDescriptionList.count());

    //sort descending according to conflicts in O(n log n)
    int ttt=conflictsCostList.count();

    QMultiMap<double, QString> map;
    for(int i=0; i<conflictsCostList.count(); i++)
        map.insert(conflictsCostList.at(i), conflictsDescriptionList.at(i));

    conflictsCostList.clear();
    conflictsDescriptionList.clear();

    QMapIterator<double, QString> i(map);
    while (i.hasNext()) {
        i.next();
        conflictsCostList.prepend(i.key());
        conflictsDescriptionList.prepend(i.value());
    }

    for(int i=0; i<conflictsCostList.count()-1; i++)
        assert(conflictsCostList.at(i) >= conflictsCostList.at(i+1));

    assert(conflictsCostList.count()==conflictsDescriptionList.count());
    assert(conflictsCostList.count()==ttt);
}

void ConstraintConflictDataForGroup::clear()
{
    conflictsCostList.clear();
    conflictsDescriptionList.clear();
    costTotal = 0;
}

ConstraintConflictData::ConstraintConflictData()
{
    ConstraintConflictDataForGroup ccdgE(Enums::ConstraintGroup::Essential);
    group_data_map.insert(Enums::ConstraintGroup::Essential, ccdgE);

    ConstraintConflictDataForGroup ccdgI(Enums::ConstraintGroup::Important);
    group_data_map.insert(Enums::ConstraintGroup::Important, ccdgI);

    ConstraintConflictDataForGroup ccdgD(Enums::ConstraintGroup::Desirable);
    group_data_map.insert(Enums::ConstraintGroup::Desirable, ccdgD);
}

QList<double> ConstraintConflictData::conflictsCostList()
{
    QList<double> list;

    list.append(conflictsCostList(Enums::ConstraintGroup::Essential));
    list.append(conflictsCostList(Enums::ConstraintGroup::Important));
    list.append(conflictsCostList(Enums::ConstraintGroup::Desirable));

    return list;
}

QList<double> ConstraintConflictData::conflictsCostList(Enums::ConstraintGroup g)
{
    return group_data_map[g].conflictsCostList;
}

QList<QString> ConstraintConflictData::conflictsDescriptionList()
{
    QList<QString> list;

    list.append(conflictsDescriptionList(Enums::ConstraintGroup::Essential));
    list.append(conflictsDescriptionList(Enums::ConstraintGroup::Important));
    list.append(conflictsDescriptionList(Enums::ConstraintGroup::Desirable));

    return list;
}

QList<QString> ConstraintConflictData::conflictsDescriptionList(Enums::ConstraintGroup g)
{
    return group_data_map[g].conflictsDescriptionList;
}

double ConstraintConflictData::costTotal()
{
    double cost = 0;

    cost += costTotal(Enums::ConstraintGroup::Essential);
    cost += costTotal(Enums::ConstraintGroup::Important);
    cost += costTotal(Enums::ConstraintGroup::Desirable);

    return cost;
}

double ConstraintConflictData::costTotal(Enums::ConstraintGroup g)
{
    return group_data_map[g].costTotal;
}

void ConstraintConflictData::addConflict(Enums::ConstraintGroup g, QString desc, double cost)
{
    group_data_map[g].conflictsDescriptionList.append(desc);
    group_data_map[g].conflictsCostList.append(cost);
}

void ConstraintConflictData::computeData()
{
    group_data_map[Enums::ConstraintGroup::Essential].computeData();
    group_data_map[Enums::ConstraintGroup::Important].computeData();
    group_data_map[Enums::ConstraintGroup::Desirable].computeData();
}

void ConstraintConflictData::clear()
{
    group_data_map[Enums::ConstraintGroup::Essential].clear();
    group_data_map[Enums::ConstraintGroup::Important].clear();
    group_data_map[Enums::ConstraintGroup::Desirable].clear();
}

SubjectPreferenceDataForLevel::SubjectPreferenceDataForLevel()
{
}

SubjectPreferenceDataForLevel::SubjectPreferenceDataForLevel(Enums::SubjectPreference l) : level(l)
{
}

void SubjectPreferenceDataForLevel::computeData()
{
}

void SubjectPreferenceDataForLevel::clear()
{
    teachersIndexes_count_map.clear();
}

SubjectPreferenceData::SubjectPreferenceData()
{
    SubjectPreferenceDataForLevel spdflVeryHigh(Enums::Very_High);
    level_data_map.insert(Enums::Very_High, spdflVeryHigh);

    SubjectPreferenceDataForLevel spdflHigh(Enums::High);
    level_data_map.insert(Enums::High, spdflHigh);

    SubjectPreferenceDataForLevel spdflIndifferent(Enums::Indifferent);
    level_data_map.insert(Enums::Indifferent, spdflIndifferent);

    SubjectPreferenceDataForLevel spdflLow(Enums::Low);
    level_data_map.insert(Enums::Low, spdflLow);

    SubjectPreferenceDataForLevel spdflVeryLow(Enums::Very_Low);
    level_data_map.insert(Enums::Very_Low, spdflVeryLow);
}

QList<int> SubjectPreferenceData::teachersIndexesList()
{
    QList<int> list;

    list.append(teachersIndexesList(Enums::Very_High));
    list.append(teachersIndexesList(Enums::High));
    list.append(teachersIndexesList(Enums::Indifferent));
    list.append(teachersIndexesList(Enums::Low));
    list.append(teachersIndexesList(Enums::Very_Low));

    return list;
}

QList<int> SubjectPreferenceData::teachersIndexesList(Enums::SubjectPreference l)
{
    return level_data_map[l].teachersIndexes_count_map.keys();
}

void SubjectPreferenceData::addPreferenceData(Enums::SubjectPreference l, int teacherIndex)
{
    QHash<int, int> &map = level_data_map[l].teachersIndexes_count_map;

    if (map.contains(teacherIndex)) {
        map[teacherIndex]++;
    }
    else{
        map.insert(teacherIndex, 1);
    }
}

void SubjectPreferenceData::computeData()
{
}

void SubjectPreferenceData::clear()
{
    level_data_map.clear();
}


Move::~Move() {}
