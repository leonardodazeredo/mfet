#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>
#include <vector>

class Enums : public QObject {
    Q_OBJECT
public:
    Enums();

    enum Status { Success, Fail, Cancel, Abort };
    Q_ENUM(Status)

    enum ConstraintGroup { Essential, Important, Desirable };
    // keep order
    Q_ENUM(ConstraintGroup)

    enum SubjectPreference { Very_High, High, Indifferent, Low, Very_Low };
    // keep order
    Q_ENUM(SubjectPreference)

    enum Neighborhood { None, N1, N2, N3, N4, N5 };
    // keep order
    Q_ENUM(Neighborhood)
    static std::vector<Neighborhood> allNeighborhoods;

    enum FromEmptySolutionStrategy { GRASP, Reactive_GRASP, Random_Start} ;
    // keep order
    Q_ENUM(FromEmptySolutionStrategy)

    enum FromInicialSolutionStrategy { VND, VNS_Ordered_Neighborhoods, VNS_Random_Neighborhood, Simulated_Annealing, Best_Improving, First_Improving };
    // keep order
    Q_ENUM(FromInicialSolutionStrategy)

    enum ConstructionMethod { Construction_By_SubGroups, Construction_By_Activities };
    Q_ENUM(ConstructionMethod)

    enum VerboseLevel { Full, Short, Minimal, Details_For_Tests };
    Q_ENUM(VerboseLevel)
};

#endif // ENUMS_H
