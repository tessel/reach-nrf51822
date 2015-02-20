PROJECT_NAME := reach

OUTPUT_FILENAME = reach
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

FLASH_START_ADDRESS = 0x16000

SDK_PATH = ./nRF51_SDK_7.1.0/
SOFTDEVICE = s110_nrf51822_7.1.0/s110_nrf51822_7.1.0_softdevice.hex

TEMPLATE_PATH = ./$(SDK_PATH)components/toolchain/gcc

GNU_INSTALL_ROOT := /usr/local
GNU_PREFIX := arm-none-eabi
GNU_VERSION := $(shell $(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc -dumpversion)

MK := mkdir
RM := rm -rf

MANUFACTURER_DATA := Technical Machine

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
endif

# Toolchain commands
CC       		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc"
AS       		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as"
AR       		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar" -r
LD       		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld"
NM       		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm"
OBJDUMP  		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump"
OBJCOPY  		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy"
SIZE    		:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size"
GDB					:= "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gdb"

JLINK = /usr/bin/JLinkExe
JLINKGDBSERVER = /usr/bin/JLinkGDBServer
GDB_DIRECTORY = /usr/bin

GDB_PORT_NUMBER := 2331

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

#source common to all targets
C_SOURCE_FILES += \
$(SDK_PATH)components/libraries/button/app_button.c \
$(SDK_PATH)components/libraries/fifo/app_fifo.c \
$(SDK_PATH)components/libraries/util/app_error.c \
$(SDK_PATH)components/libraries/timer/app_timer.c \
$(SDK_PATH)components/libraries/gpiote/app_gpiote.c \
$(SDK_PATH)components/libraries/scheduler/app_scheduler.c \
$(SDK_PATH)components/libraries/util/nrf_assert.c \
$(SDK_PATH)components/drivers_nrf/uart/app_uart_fifo.c \
$(SDK_PATH)components/drivers_nrf/hal/nrf_delay.c \
src/main.c \
src/gossip.c \
src/i2c.c \
$(SDK_PATH)components/softdevice/common/softdevice_handler/softdevice_handler.c \
$(SDK_PATH)components/toolchain/system_nrf51.c \
$(SDK_PATH)components/ble/common/ble_conn_params.c \
$(SDK_PATH)components/ble/common/ble_advdata.c \
$(SDK_PATH)components/ble/common/ble_srv_common.c \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(SDK_PATH)components/toolchain/gcc/gcc_startup_nrf51.s

#includes common to all targets
INC_PATHS += -I$(SDK_PATH)components/toolchain/gcc
INC_PATHS += -I$(SDK_PATH)components/toolchain
INC_PATHS += -I$(SDK_PATH)components/libraries/button
INC_PATHS += -I$(SDK_PATH)components/softdevice/s110/headers
INC_PATHS += -I$(SDK_PATH)components/ble/common
INC_PATHS += -I$(SDK_PATH)components/drivers_nrf/uart
INC_PATHS += -I$(SDK_PATH)components/libraries/fifo
INC_PATHS += -I$(SDK_PATH)components/libraries/timer
INC_PATHS += -I$(SDK_PATH)components/libraries/gpiote
INC_PATHS += -I$(SDK_PATH)components/drivers_nrf/hal
INC_PATHS += -I$(SDK_PATH)components/softdevice/common/softdevice_handler
INC_PATHS += -I$(SDK_PATH)components/libraries/scheduler
INC_PATHS += -I$(SDK_PATH)components/libraries/util

OBJECT_DIRECTORY = obj
LISTING_DIRECTORY = bin
OUTPUT_BINARY_DIRECTORY = build

ELF := $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF51
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DS110
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DBOARD_PCA10028
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror -O3
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -flto -fno-builtin
CFLAGS += -ggdb
CFLAGS += -D MANUFACTURER_DATA="\"$(MANUFACTURER_DATA)\""

# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF51
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DS110
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DBOARD_PCA10028
#default target - first one defined
default: clean $(OUTPUT_FILENAME)
	
#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e $(OUTPUT_FILENAME)
	
#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf51422_xxaa_s110
	
	
C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )
	
ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )
	
vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)
	
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

$(OUTPUT_FILENAME): LINKER_SCRIPT=$(SDK_PATH)components/toolchain/gcc/gcc_nrf51_s110_xxaa.ld
$(OUTPUT_FILENAME): $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize
	
## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@
	
# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<
	
# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<
	
	
# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -c -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	
	
## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin
	
## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
	
finalize: genbin genhex echosize
	
genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin
	
## Create binary .hex file from the .out file
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
	
echosize:
	-@echo ""
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ""
	
clean:
	$(RM) $(BUILD_DIRECTORIES)
	
cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o

# softdevice-objects:
# 	$(OBJCOPY) -Iihex -Oelf32-littlearm -B arm --remove-section .sec3 $(SOFTDEVICE) $(OUTPUT_BINARY_DIRECTORY)/_mainpart.o
# 	$(OBJCOPY) -Iihex -Oelf32-littlearm -B arm --remove-section .sec1 --remove-section .sec2 $(SOFTDEVICE) $(OUTPUT_BINARY_DIRECTORY)/_uicr.o
# 	# $(CC) -T config/sd_s110.ld $(OUTPUT_BINARY_DIRECTORY)/_mainpart.o $(OUTPUT_BINARY_DIRECTORY)/_uicr.o -o $(OUTPUT_BINARY_DIRECTORY)/softdevice.out
# 
# # OpenOCD Flash Targets
# flash-softdevice:
# 	$(OBJCOPY) -Iihex -Oelf32-littlearm --remove-section .sec3 $(SOFTDEVICE) $(OUTPUT_BINARY_DIRECTORY)/_mainpart.o
# 	$(OBJCOPY) -Iihex -Oelf32-littlearm --remove-section .sec1 --remove-section .sec2 $(SOFTDEVICE) $(OUTPUT_BINARY_DIRECTORY)/_uicr.o
# 	# openocd -f nrf.cfg

# Program device
flash: rm-flash.jlink flash.jlink
	$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/flash.jlink || true

flash.jlink:
	echo "device nrf51822\nspeed 1000\nr\nloadbin $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin, $(FLASH_START_ADDRESS)\nr\ng\nexit\n" > $(OUTPUT_BINARY_DIRECTORY)/flash.jlink
	

rm-flash.jlink:
	-rm -rf $(OUTPUT_BINARY_DIRECTORY)/flash.jlink

recover: recover.jlink erase-all.jlink pin-reset.jlink
	-$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/recover.jlink
	-$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/erase-all.jlink
	-$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/pin-reset.jlink
	
recover.jlink:
	echo "device nrf51822\nspeed 1000\nsi 0\nt0\nsleep 1\ntck1\nsleep 1\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\nt0\nsleep 2\nt1\nsleep 2\ntck0\nsleep 100\nsi 1\nr\nexit\n" > $(OUTPUT_BINARY_DIRECTORY)/recover.jlink
	
pin-reset.jlink:
	echo "device nrf51822\nspeed 1000\nw4 4001e504 2\nw4 40000544 1\nr\nexit\n" > $(OUTPUT_BINARY_DIRECTORY)/pin-reset.jlink

erase-all: erase-all.jlink
	$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/erase-all.jlink || true
	
erase-all.jlink:
	echo "device nrf51822\nw4 4001e504 2\nw4 4001e50c 1\nsleep 100\nw4 4001e504 1\nr\nexit\n" > $(OUTPUT_BINARY_DIRECTORY)/erase-all.jlink


flash-softdevice: erase-all flash-softdevice.jlink
ifndef SOFTDEVICE
	${error "You need to set the SOFTDEVICE command-line parameter to a path (without spaces) to the softdevice hex-file"}
endif
	
	# Convert from hex to binary. Split original hex in two to avoid huge (>250 MB) binary file with just 0s. 
	$(OBJCOPY) -Iihex -Obinary $(SOFTDEVICE) $(OUTPUT_BINARY_DIRECTORY)/softdevice.bin
	
	-$(JLINK) $(OUTPUT_BINARY_DIRECTORY)/flash-softdevice.jlink

flash-softdevice.jlink:
	echo "device nrf51822\nspeed 1000\nw4 4001e504 1\nloadbin \"$(OUTPUT_BINARY_DIRECTORY)/softdevice.bin\" 0\nr\ng\nexit\n" > $(OUTPUT_BINARY_DIRECTORY)/flash-softdevice.jlink

startdebug: stopdebug debug.jlink .gdbinit
	$(JLINKGDBSERVER) -if swd -speed 1000 -port $(GDB_PORT_NUMBER) &
	sleep 3
	$(GDB) $(ELF)

stopdebug:
	-killall $(JLINKGDBSERVER)

.gdbinit:
	echo "target remote localhost:$(GDB_PORT_NUMBER)\nmonitor flash download = 1\nmonitor flash device = nrf51822\nbreak main\nmon reset\n" > .gdbinit

debug.jlink:
	echo "Device nrf51822" > $(OUTPUT_BINARY_DIRECTORY)/debug.jlink

test:
	node test/loopback.js

.PHONY: flash flash-softdevice erase-all startdebug stopdebug test
