M109 S240.000000
;Sliced at: Mon 19-12-2016 17:50:27
;Basic settings: Layer height: 0.2 Walls: 1 Fill: 20
;Print time: 0 minutes
;Filament used: 0.0m 0.0g
;Filament cost: None
;M190 S70 ;Uncomment to add your own bed temperature line
;M109 S240 ;Uncomment to add your own temperature line
G21        ;metric values
G90        ;absolute positioning
M82        ;set extruder to absolute mode
M107       ;start with the fan off
G28 X0 Y0  ;move X/Y to min endstops
G28 Z0     ;move Z to min endstops
G1 Z15.0 F9000 ;move the platform down 15mm
G92 E0                  ;zero the extruded length
G1 F200 E3              ;extrude 3mm of feed stock
G92 E0                  ;zero the extruded length again
G1 F9000
;Put printing message on LCD screen
M117 Printing...

;Layer count: 8
;LAYER:0
M107
;LAYER:1
M106 S255
;LAYER:2
;LAYER:3
;LAYER:4
;LAYER:5
;LAYER:6
;LAYER:7
M107
G1 F2400 E-4.50000
G0 F9000 X0.000 Y0.000 Z6.738
;End GCode
M104 S0                     ;extruder heater off
M140 S0                     ;heated bed heater off (if you have it)
G91                                    ;relative positioning
G1 E-1 F300                            ;retract the filament a bit before lifting the nozzle, to release some of the pressure
G1 Z+0.5 E-5 X-20 Y-20 F9000 ;move Z up a bit and retract filament even more
G28 X0 Y0                              ;move X/Y to min endstops, so the head is out of the way
M84                         ;steppers off
G90                         ;absolute positioning
;CURA_PROFILE_STRING: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

