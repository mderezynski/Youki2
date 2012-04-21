#include <vector>
#include <limits>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "mpx/algorithm/random.hh"

namespace MPX
{
#define ARRAY_SIZE(array) (sizeof (array) / sizeof ((array)[0]))
void compute_intervals (std::vector<double>& intervals, std::vector<double> const& ratios)
{
    intervals.clear ();

    if (ratios.empty ())
        return;

    intervals.reserve (ratios.size ());

    double sum = 0;
    for (unsigned int i = 0; i < ratios.size (); i++)
        sum += ratios[i];

    double bound = 0.0;
    for (unsigned int i = 0; i < ratios.size (); i++)
    {
        bound += ratios[i] / sum;
        intervals.push_back (bound);
    }
}

unsigned int random_element (std::vector<double> const& intervals)
{
    double x = double (std::rand ()) / std::numeric_limits<int>::max();

    int lower = 0;
    int upper = intervals.size () - 1;

    while (lower < upper)
    {
        int middle = (lower + upper) / 2;

        if (x <= intervals[middle])
            upper = middle;
        else
            lower = middle + 1;
    }

    return upper;
}

int rand (const std::vector<double>& ratios)
{
    std::srand (std::time (0));
    std::vector<double> intervals;
    compute_intervals (intervals, ratios);
    return random_element(intervals);
}
}
