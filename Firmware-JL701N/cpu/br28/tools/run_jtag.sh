#!/bin/sh
#sleep 4

#-ex "detach" -ex "q"
/opt/gdb $1 -ex "target remote proxy:"${NICKNAME}"_jtag:9872" -ex "b exception_irq_handler" -ex "c" 
