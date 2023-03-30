#ifndef __MATH_H__
#define __MATH_H__

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))
#define is_power_of_two(x) (((x) & ((x) - 1)) == 0)

#endif /*__MATH_H__*/