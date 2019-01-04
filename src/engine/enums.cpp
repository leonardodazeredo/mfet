#include "enums.h"

std::vector<Enums::Neighborhood> Enums::allNeighborhoods;

Enums::Enums()
{
    this->allNeighborhoods = { N1, N2, N3, N4, N5 };// keep order
}
