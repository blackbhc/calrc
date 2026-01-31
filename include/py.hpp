#include "grid.hpp"
#include <vector>

extern "C" {
double* calAccRs(int           np,
                 // NOLINTBEGIN
                 const double* sourceMasses,
                 const double* sourceCoordinates,
                 // NOLINTEND
                 int           numGridPoint,
                 const double* gridCoords,
                 int           numThread = 1);

void release(void);
}
