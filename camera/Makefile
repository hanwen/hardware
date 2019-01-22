PACKDIR=/home/hanwen/.arduino15/packages
ARDDIR=/home/hanwen/vc/arduino-1.8.8

flash: build
	esptool --baud 460800 --port /dev/ttyUSB0  write_flash --flash_mode dio --flash_size detect 0x0 .out/camera.ino.bin
	socat /dev/ttyUSB0,raw,echo=0,crnl,b115200 -

build:
	arduino-builder \
		-compile \
		-logger=machine \
		-hardware $(ARDDIR)/hardware \
		-hardware /home/hanwen/.arduino15/packages \
		-tools $(ARDDIR)/tools-builder \
		-tools $(ARDDIR)/hardware/tools/avr \
		-tools /home/hanwen/.arduino15/packages \
		-built-in-libraries $(ARDDIR)/libraries \
		-libraries ../libraries \
		-fqbn=esp8266:esp8266:nodemcuv2:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=230400 \
		-ide-version=10808 \
		-build-path $(PWD)/.out/ \
		-warnings=none \
		-build-cache $(PWD)/.cache/ \
		-prefs=build.warn_data_percentage=75 \
		-prefs=runtime.tools.mkspiffs.path=$(PACKDIR)/esp8266/tools/mkspiffs/0.2.0 \
		-prefs=runtime.tools.mkspiffs-0.2.0.path=$(PACKDIR)/esp8266/tools/mkspiffs/0.2.0 \
		-prefs=runtime.tools.esptool.path=$(PACKDIR)/esp8266/tools/esptool/0.4.13 \
		-prefs=runtime.tools.esptool-0.4.13.path=$(PACKDIR)/esp8266/tools/esptool/0.4.13 \
		-prefs=runtime.tools.xtensa-lx106-elf-gcc.path=$(PACKDIR)/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 \
		-prefs=runtime.tools.xtensa-lx106-elf-gcc-1.20.0-26-gb404fb9-2.path=$(PACKDIR)/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 \
		camera.ino


clean:
	rm -rf .cache .out ; mkdir .out .cache
