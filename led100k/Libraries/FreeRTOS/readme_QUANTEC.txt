Änderungen und Modifikationen
=============================

2011/03/21 und davor, CBA
-------------------------
   portable/GCC/ARM_CM3/portmacro.h und ARM_CM3_MPU/portmacro.h
   SysTickTimer auf 64bit erweitert. configUSE_64_BIT_TICKS 
   !!! Dies muss unbedingt bei einem Update übernommen werden, sonst HardFault !!!

   portable/GCC/ARM_CM3_MPU/port.c
   In Funktion prvSetupMPU muss extern unsigned long __privileged_functions_start__[]; ergänzt werden.
   Ausserdem muss __privileged_functions_start__ als portMPU_REGION_BASE_ADDRESS übergeben werden.
   
   Dann gibts noch einen Haufen Warnings zu fixe. Da muss man ein Diff machen und sich durcharbeiten.
   