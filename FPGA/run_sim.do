# ==============================================================================
# ModelSim Automated Verification Script — Porygon FPGA Project 2026
# Usage: Open ModelSim, cd to FPGA directory, and run: do run_sim.do
# ==============================================================================

onerror {resume}
transcript on

# 1. Create and map working library
if {[file exists work]} {
    vdel -lib work -all
}
vlib work
vmap work work

# 2. Compile Gowin Primitive Simulation Library & Synthesizable RTL
vlog src/prim_sim.v
vlog src/gowin_pllvr.v
vlog src/button_debounce.v
vlog src/pwm_led_controller.v
vlog src/uart_tx_string.v
vlog src/top_system.v

# 3. Compile Automated Verification Testbench
vlog sim/tb_top_system_v2.v

# 4. Initialize Simulation with Full Visibility (+acc)
vsim -voptargs="+acc" work.tb_top_system_v2

# 5. Load Waveform & Cursors
if {[file exists wavefinal.do]} {
    do wavefinal.do
} elseif {[file exists sim/wavefinal.do]} {
    do sim/wavefinal.do
}

# 6. Execute Simulation
run -all
