#ifndef _iv_vector_h
#define _iv_vector_h
#define _min_save min
#define _max_save max
#undef min
#undef max
#include_next <iterator>
#include_next <vector.h>
#define min _min_save
#define max _max_save
#endif
