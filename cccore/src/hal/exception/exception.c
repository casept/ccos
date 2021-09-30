#include "../../include/exception.h"

extern void exception_gpf_register_default(void);

void exception_register_default(void) { exception_gpf_register_default(); }
