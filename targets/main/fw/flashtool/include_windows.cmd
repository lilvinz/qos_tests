
set OPENOCD_COMMANDLINE=-f interface/ftdi/olimex-arm-usb-tiny-h.cfg -f target/stm32f2x.cfg -f flash.oocd

set VERSION_CHECK_URL=http://master-001.build.quantec-networks.de:8080/job/sirius_r2_release/lastSuccessfulBuild/artifact/%VERSION%.tar.bz2
set VERSION_UPDATE_URL=http://master-001.build.quantec-networks.de:8080/job/sirius_r2_release/

