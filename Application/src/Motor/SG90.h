/*
 * SG90.h
 *
 * Created: 12/4/2023 7:04:13 PM
 *  Author: kenzh
 */ 
#ifndef __SG90_H
#define __SG90_H
#include "tcc.h"
#include "main.h"
#include <stdbool.h>

#include <stdint.h>

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif

#define MOTOR_STATUS_CLOSE     0 
#define MOTOR_STATUS_OPEN     1 

#endif

//int32_t MOTOR_init(void);
//int32_t MOTOR_RUN(uint8_t status);
void tcc_pwm_init(Tcc *const tcc_module, const uint32_t channel, const uint32_t lock_state);
void setup_pwm_pin(const uint32_t pin, const uint32_t mux_function);
void configure_pwm(void);
void unlock_pwm(void);
void lock_pwm(void);