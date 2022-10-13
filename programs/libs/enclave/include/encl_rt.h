#ifndef __INCLUDE_ENCL_RT_H__
#define __INCLUDE_ENCL_RT_H__

// Define the tyche_encl_handle_t here to avoid dep on the driver.
typedef unsigned long tyche_encl_handle_t;

void transition(tyche_encl_handle_t handle);

#endif
