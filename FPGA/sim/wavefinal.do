onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -height 40 /tb_top_system_v2/uut/btn1_in
add wave -noupdate -color Pink -height 40 /tb_top_system_v2/uut/btn1_pulse
add wave -noupdate -height 40 /tb_top_system_v2/uut/btn2_in
add wave -noupdate -color {Olive Drab} -height 40 /tb_top_system_v2/uut/btn2_pulse
add wave -noupdate -color Red -height 40 /tb_top_system_v2/uut/led_out
add wave -noupdate -color Gold -height 40 /tb_top_system_v2/uut/uart_tx
add wave -noupdate -color Magenta -height 40 -radix unsigned /tb_top_system_v2/uut/u_uart/bit_cnt
add wave -noupdate -height 40 -radix unsigned /tb_top_system_v2/uut/current_mode
add wave -noupdate /tb_top_system_v2/uut/u_pwm/breath_duty
add wave -noupdate /tb_top_system_v2/uut/u_pwm/breath_dir
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 4} {5158015734738 ps} 0} {{Cursor 5} {2169707034738 ps} 0} {{Cursor 6} {3165809934738 ps} 0} {{Cursor 7} {4161912834738 ps} 0} {{Cursor 8} {1173604134738 ps} 0}
quietly wave cursor active 5
configure wave -namecolwidth 293
configure wave -valuecolwidth 136
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {251422075487 ps} {5623369447035 ps}
