# ==============================================================================
# ModelSim Automated Verification Script — Porygon FPGA Project 2026
# Usage: 
#   Option A: Open ModelSim, cd to FPGA/sim directory, run: do run_sim.do
#   Option B: Open ModelSim, cd to FPGA directory, run: do sim/run_sim.do
# ==============================================================================

onerror {resume}
transcript on

# 1. Detect working directory
set SRC_DIR "../src"
set SIM_DIR "."

if {[file exists "src/top_system.v"]} {
    # Executed from FPGA root directory
    set SRC_DIR "src"
    set SIM_DIR "sim"
} elseif {[file exists "../src/top_system.v"]} {
    # Executed from FPGA/sim directory
    set SRC_DIR "../src"
    set SIM_DIR "."
}

# 2. Create and map working library
if {[file exists work]} {
    vdel -lib work -all
}
vlib work
vmap work work

# 3. Compile Gowin Primitive Simulation Library & Synthesizable RTL
vlog $SRC_DIR/prim_sim.v
vlog $SRC_DIR/gowin_pllvr.v
vlog $SRC_DIR/button_debounce.v
vlog $SRC_DIR/pwm_led_controller.v
vlog $SRC_DIR/uart_tx_string.v
vlog $SRC_DIR/top_system.v

# 4. Compile Automated Verification Testbench
vlog $SIM_DIR/tb_top_system_v2.v

# 5. Initialize Simulation with Full Visibility (+acc)
vsim -voptargs="+acc" work.tb_top_system_v2

# 6. Load Waveform & Cursors
if {[file exists "$SIM_DIR/wavefinal.do"]} {
    do $SIM_DIR/wavefinal.do
} elseif {[file exists "wavefinal.do"]} {
    do wavefinal.do
}

# 7. Execute Simulation
run -all
