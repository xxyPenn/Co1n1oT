/*
 * SG90.c
 *
 * Created: 12/1/2023 5:19:06 PM
 *  Author: Qiming Zhang
 *
 * We will choose for [PWM(+)] PA10 TCC0/WO[2] to generate the pwm signal ouput
 */ 

//#include <asf.h> // Atmel Software Framework (ASF)
#include "SG90.h"
#include "math.h"
#include "tcc.h"
#include "port.h"
#include "SerialConsole.h"


	struct tcc_module MOTOR_PWM_tcc_instance;
	struct tcc_config MOTOR_PWM_tcc;

// Function to initialize TCC for PWM
void tcc_pwm_init(Tcc *const tcc_module, const uint32_t channel, const uint32_t lock_state) {	
	// Enable the clock for the TCC module to prevent power manager from disabling it
	//system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_TCC0);

	tcc_disable(&MOTOR_PWM_tcc_instance);
	
	// Initialize the configuration structure to default values
	tcc_get_config_defaults(&MOTOR_PWM_tcc,TCC0);

	// Set the counter size to 16 bits
	//MOTOR_PWM_tcc.counter_size = TCC_COUNTER_SIZE_16BIT;
	// Set the waveform generation mode to Single-Slope PWM
	//MOTOR_PWM_tcc.wave.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM; 
	// Set the prescaler value (adjust based on clock and desired frequency)
	
	MOTOR_PWM_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;

	// Calculate the period for the PWM frequency
	//uint32_t max_count = 0x08EC;
	
	uint32_t max_count = 0xFFFF*14.65 ; //14.65 -20ms

	//LogMessage(LOG_ERROR_LVL, "frequency hz = %d!\r\n",max_count);
	MOTOR_PWM_tcc.counter.period = max_count;

	// Configure duty cycle as a percentage of the period
	
	if (lock_state == MOTOR_STATUS_OPEN){
		MOTOR_PWM_tcc.compare.match[1] = max_count*1.5/20;
		}//unlock
	
	if (lock_state == MOTOR_STATUS_CLOSE){
		MOTOR_PWM_tcc.compare.match[1] = max_count*3/20;
	}//lock
	
	/*
	MOTOR_PWM_tcc.compare.match[1] = (max_count * duty_cycle) / 100;
	MOTOR_PWM_tcc.compare.match[0] = (max_count * duty_cycle) / 100;	*/
	MOTOR_PWM_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	//MOTOR_PWM_tcc.compare.wave_ramp = TCC_RAMP_RAMP2;
	
	MOTOR_PWM_tcc.pins.enable_wave_out_pin[1] = true;
	MOTOR_PWM_tcc.pins.wave_out_pin[1] = PIN_PA05E_TCC0_WO1;
	MOTOR_PWM_tcc.pins.wave_out_pin_mux[1] = MUX_PA05E_TCC0_WO1;
	
	//MOTOR_PWM_tcc.compare.wave_polarity[0] = 1;
	//MOTOR_PWM_tcc.compare.wave_polarity[1] = 1;

			
	tcc_init(&MOTOR_PWM_tcc_instance, TCC0, &MOTOR_PWM_tcc);
	// Initialize the TCC module with the specified settings, 0 is tcc0

	// Enable the TCC module
	tcc_enable(&MOTOR_PWM_tcc_instance);

}

// Main function to configure PWM
void configure_pwm(void) {
		// Initialize TCC0 for PWM on Channel 2 with 50Hz frequency and 10% duty cycle
	
		SerialConsoleWriteString("Now start to configure PWM_Motor pin!\r\n");
		
		tcc_pwm_init(TCC0, 0, MOTOR_STATUS_CLOSE);// reset MOTOR_CLOSE

		SerialConsoleWriteString("PWM pin configure finished!\r\n");
}

void unlock_pwm(void) {
	// Initialize TCC0 for PWM on Channel 2 with 50Hz frequency and 10% duty cycle
		
	tcc_pwm_init(TCC0, 0, MOTOR_STATUS_OPEN);// unlock state motor

}
void lock_pwm(void) {
	// Initialize TCC0 for PWM on Channel 2 with 50Hz frequency and 10% duty cycle
	
	tcc_pwm_init(TCC0, 0, MOTOR_STATUS_CLOSE);// lock motor
}

// Call configure_pwm in your main function or setup routine