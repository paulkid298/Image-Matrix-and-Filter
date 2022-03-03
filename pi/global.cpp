#include "global.hpp"

int ready(int ind_curr, int ind_parent)
{
    if ((ind_curr + 1) % NUM_IMAGES == ind_parent)
    {
        return 0;
    }
    return 1;
}
