target remote localhost:3333
set remotetimeout 60
set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4

define hook-step
mon cortex_m3 maskisr on
end
define hookpost-step
mon cortex_m3 maskisr off
end

monitor reset halt


