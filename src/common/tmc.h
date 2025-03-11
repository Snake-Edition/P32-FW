// trinamic.h
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct {
    const char *cmd_name;
    uint8_t reg_adr;
    bool write;
    bool read;
} tmc_reg_t;

extern tmc_reg_t tmc_reg_map[]; //< Null terminated array of known registers

extern void init_tmc_bare_minimum(void);
extern void init_tmc(void);
extern void tmc_get_sgt();
extern void tmc_get_TPWMTHRS();
extern void tmc_get_tstep();
extern uint16_t tmc_sg_result(uint8_t axis);

extern void tmc_enable_wavetable(bool X, bool Y, bool Z);
extern void tmc_disable_wavetable(bool X, bool Y, bool Z);

/**
 * \brief Check stepper coils for open/short circuit
 *
 * This reports false errors when not moving or moving too fast.
 *
 * \param axis axis to check
 * \return true if all coils are ok, false otherwise
 */
extern bool tmc_check_coils(uint8_t axis);
#ifdef __cplusplus
}

#endif //__cplusplus
