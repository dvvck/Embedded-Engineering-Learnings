#![no_std]
#![no_main]

use core::panic::PanicInfo;

use cortex_m::asm;
use cortex_m_semihosting::hprintln;

//use cortex_m::peripheral::syst::SystClkSource;
//use cortex_m::peripheral::Peripherals;
use cortex_m_rt::{entry, exception};

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

// System tick - control and status register
const SYST_CSR: u32 = 0xE000E010;
// System tick - reload value register
const SYST_RVR: u32 = 0xE000E014;
// System tick - current value register
const SYST_CVR: u32 = 0xE000E018;
// CPU frequency (12.5 MHz by default)
const CPU_FREQ: u32 = 12_500_000;

#[entry]
fn main() -> ! {
    hprintln!("Staring our program!");

    //let peripherals = Peripherals::take().unwrap();
    //let mut systick = peripherals.SYST;
    //systick.enable_interrupt();
    //systick.set_clock_source(SystClkSource::Core);
    //systick.set_reload(CPU_FREQ);
    //systick.clear_current();
    //systick.enable_counter();

    unsafe {
        let sleep_dur: u32 = CPU_FREQ;
        // Set the timer duration
        *(SYST_RVR as *mut u32) = sleep_dur;
        // Clear the current value by writing to the register
        *(SYST_CVR as *mut u32) = 0;
        // Enable the clock
        *(SYST_CSR as *mut u32) = 0b111;
    }

    loop {
        asm::wfi();
    }
}

#[exception]
fn SysTick() {
    hprintln!("ugh, woke up :(")
}
