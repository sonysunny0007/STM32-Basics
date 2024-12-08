Global Variables
hadc1, htim3, huart2, hdma_adc1: Handle structures for ADC, Timer, UART, and DMA, respectively.
adc_buffer[1]: Buffer to hold the ADC conversion result.
uart_buffer[50]: Buffer for UART transmissions.
adc_buf[ADC_BUF_LEN]: ADC buffer for DMA transfer, declared as volatile because it is accessed in interrupt contexts.

Main Function
HAL_Init(): Initializes the HAL library.
SystemClock_Config(): Configures the system clock for the microcontroller.
UART Message: Outputs a startup message over UART for debugging.

Initialization:
MX_GPIO_Init(): Sets up GPIO pins for ADC input and PWM output.
DMA_Init(): Configures the DMA controller for ADC data transfer.
UART_Init(): Configures UART for serial communication.
MX_ADC_Init(): Configures the ADC for single-channel conversion with DMA.
MX_TIM_Init(): Configures Timer 3 for PWM and TRGO (trigger output) functionality.

Peripheral Starts:
HAL_TIM_Base_Start(): Starts Timer 3 in base mode.
HAL_TIM_PWM_Start_IT(): Starts PWM on Timer 3, Channel 3 with interrupts.
HAL_ADC_Start_DMA(): Starts the ADC in DMA mode, writing conversions to adc_buf.

Infinite Loop:
Reads the ADC value, converts it to voltage, and displays it via UART.
Adjusts the PWM duty cycle based on the ADC value.
GPIO Initialization (MX_GPIO_Init)

Configures:
PA0 as analog input for the ADC.
PB0 as an alternate function pin for PWM output using TIM3 Channel 3.
DMA Initialization (DMA_Init)
Sets up DMA for peripheral-to-memory transfers.
Enables circular mode for continuous ADC data transfer.
Links the DMA handle to the ADC handle (__HAL_LINKDMA).
ADC Initialization (MX_ADC_Init)
Configures the ADC to:
Use 12-bit resolution.
Trigger conversions from Timer 3's TRGO signal.
Transfer data via DMA.
Use Channel 0 (PA0) for input.
Sampling time is set to 144 ADC clock cycles for stability.
Timer Initialization (MX_TIM_Init)

Configures Timer 3:
Base Timer: Generates a 10 Hz TRGO signal for the ADC.
PWM Timer: Controls the duty cycle on TIM3 Channel 3 for PWM output.
Enables TRGO output for ADC synchronization.
PWM Duty Cycle Adjustment (PWM_SetDutyCycle)
Adjusts the PWM duty cycle as a percentage of the timer period.
Converts ADC readings to a percentage value for brightness control.
ADC Conversion Complete Callback (HAL_ADC_ConvCpltCallback)
Triggered after an ADC conversion completes.
Converts the ADC value to voltage and transmits it via UART.
System Tick Handler
Increments the HAL system tick for internal timing functions.

Key Functionalities
ADC with DMA: Efficiently transfers ADC data to memory without CPU intervention.
PWM Output: Adjusts the duty cycle of a PWM signal based on ADC readings.
UART Communication: Transmits real-time voltage readings over serial.
Timer and TRGO: Timer generates periodic triggers for ADC conversions.

Summary
This project combines ADC and PWM functionalities using DMA for efficient data transfer and UART for debugging. It demonstrates how to dynamically control PWM duty cycles based on real-time analog input, making it suitable for applications like motor control, LED dimming, or sensor-driven systems.