#include <math_ops.h>

size_t sat_subsz(size_t a, size_t b) {
    size_t res = a + b;
    res |= -(res < a);

    return res;
}
