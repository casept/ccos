#include "../../include/exception.h"

extern void exception_gpf_register_default(void);
extern void exception_pf_register_default(void);
extern void exception_ud_register_default(void);

void exception_register_default(void) {
    exception_gpf_register_default();
    exception_pf_register_default();
    exception_ud_register_default();
}
