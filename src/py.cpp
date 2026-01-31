#include "py.hpp"
#include "grid.hpp"
#include <array>
#include <vector>


namespace {
// for coordinates, avoid magic number
constexpr int vecDim = 3;

// the global data holder
// NOLINTBEGIN
double* data{nullptr};
// NOLINTEND
}  // namespace

extern "C" {
double* calAccRs(int           np,
                 // NOLINTBEGIN
                 const double* sourceMasses,
                 const double* sourceCoordinates,
                 // NOLINTEND
                 int           numGridPoint,
                 const double* gridCoords,
                 int           numThread)
{
    ( void )numThread;
    // NOLINTBEGIN
    data = new double[numGridPoint];
    std::vector<double>                masses(sourceMasses, sourceMasses + np);
    // copy the data
    std::vector<std::array<double, 3>> coordinates(np, {0, 0, 0});
    for (int i = 0; i < np; ++i)
    {
        coordinates[i][0] = sourceCoordinates[i * vecDim + 0];
        coordinates[i][1] = sourceCoordinates[i * vecDim + 1];
        coordinates[i][2] = sourceCoordinates[i * vecDim + 2];
    }
    // NOLINTEND

// accelerating the calculation with OpneMP
#pragma omp parallel for num_threads(numThread) default(none) \
    shared(data, masses, coordinates, numGridPoint, gridCoords)
    for (int i = 0; i < numGridPoint; ++i)  // i must be int for openmp to work
    {
        // NOLINTBEGIN
        GridPoint point(gridCoords[i * vecDim + 0], gridCoords[i * vecDim + 1],
                        gridCoords[i * vecDim + 2]);
        // get the accR at each grid point
        data[i] = point.accR_from(masses, coordinates);  // with default G
        // NOLINTEND
    }
    return data;
}

void release(void)
{
    // NOLINTBEGIN
    if (data == nullptr)
        return;
    delete[] data;
    data = nullptr;
    // NOLINTEND
}
}
