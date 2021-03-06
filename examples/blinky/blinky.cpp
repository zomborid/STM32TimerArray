#include "app.h"

// include pin naming
#include "main.h"

// include setup timer handles
#include "tim.h"

#include "STM32TimerArray.hpp"

// Timer hardware's input frequency, set in code or CubeMX
// In this example we use the CPU's frequency (what was set in CubeMX by default).
uint32_t timerInputFrequency = F_CPU;

// The timers targeted frequency division, might not be achievable with the available hardware.
// Divides the |timerInputFrequency|, the result is the controller's tick frequency.
// To get the correct division, we divided the input frequency with the target frequency.
// In this example the target frequency is 10 kHz or 10000 Hz.
// *** Warning: this is integer division, the result will be rounded towards negative infinity. ***
uint32_t frequencyDivision = timerInputFrequency/10000;

// Number of usable bits for counting in the timer. Some timers have 16 bits, others have 32 bit counter register.
// Using 16 bits for a 32 bit counter will be OK, but severly limits the maximum possible delay.
uint32_t timerCounterBits = 16;

// instantiate the controller
TimerArrayControl control(
    &htim2, // handle for the used timer hardware, setup by CubeMX
    timerInputFrequency,
    frequencyDivision,
    timerCounterBits);


// Callback function (assumes that LD2 is the user LED, which is set in CubeMX)
void toggle_led(){
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

Timer t_toggle(
    5000,      // the number of ticks to wait
    true,      // isPeriodic, false means one shot, true means repeating
    toggle_led // pointer to the static callback function
);

void stop_toggle(){
    // stop toggle timer
    control.detachTimer(&t_toggle);

    // turn off LED
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

Timer t_stop(
    50000, // 5 sec delay, since |timerCounterBits| was set to 16, the maximum delay is 6.5 seconds (65535 ticks)
    false, // one shot timer, when fires, it is automatically detached
    stop_toggle
);

void app_start(){

    
    // Attach before begin is also valid and a good way to synchronize timers at startup.
    // control.attachTimer(&t_toggle);

    // Start timer array configured to run at 10 kHz (tick frequency).
    control.begin();

    // Initiate half second toggle, meaning 1 Hz blinking.
    // 0.1 ms (millisecond) per tick counting multiplied by 5000 ticks is 500 ms or 0.5 sec.
    control.attachTimer(&t_toggle);

    // Initiate stop of blinking, use a special function to synchronize timer callbacks
    control.attachTimerInSync(&t_stop, &t_toggle);
    
    // This would work too synchronization in this case is not necessary.
    // control.attachTimer(&t_stop);
    
    // Lay back, the timer callbacks are set, interrupts will handle everything.
    while(1);
}