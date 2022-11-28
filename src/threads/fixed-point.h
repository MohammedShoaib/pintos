/* ##> Our implementation */
#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define P 17
#define Q 14
#define FRACTION 1 << (Q)

#define CONVERT_TO_FP(n) (n) * (FRACTION)
#define CONVERT_TO_INT_ZERO(x) (x) / (FRACTION)
#define CONVERT_TO_NEAREST_INT(x) ((x) >= 0 ? ((x) + (FRACTION) / 2)\
                                   / (FRACTION) : ((x) - (FRACTION) / 2)\
                                   / (FRACTION))
#define ADD(x, y) (x) + (y)
#define SUB(x, y) (x) - (y)
#define ADD_INT(x, n) (x) + (n) * (FRACTION)
#define SUB_INT(x, n) (x) - (n) * (FRACTION)
#define MULTIPLY(x, y) ((int64_t)(x)) * (y) / (FRACTION)
#define MULTIPLY_INT(x, n) (x) * (n)
#define DIVIDE(x, y) ((int64_t)(x)) * (FRACTION) / (y)
#define DIVIDE_INT(x, n) (x) / (n)

#endif