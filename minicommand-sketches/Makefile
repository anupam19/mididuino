PDEDIR ?= .

CC=avr-gcc
CXX=avr-g++
LLVMCC=avr-llvm-gcc
LLVMCXX=avr-llvm-g++
OBJCOPY = avr-objcopy
UISP=uisp
AVR_ARCH = atmega64
F_CPU = 16000000L

CORE = minicommand2
#BASE_DIR = /Applications/Mididuino.app
BASE_DIR ?= ../../build/macosx/work/Mididuino.app/Contents/Resources/Java
HARDWARE_DIR = $(BASE_DIR)/hardware

LBASE_DIR = $(HARDWARE_DIR)/libraries
CORE_DIR = $(HARDWARE_DIR)/cores/$(CORE)
SEPARATOR = $(shell if test -e C:/; then echo ";"; else echo ":"; fi)
CLASSPATH = $(subst jar ,jar$(SEPARATOR),$(wildcard $(BASE_DIR)/lib/*.jar) $(wildcard $(BASE_DIR)/*.jar))

PDEFILES=$(wildcard $(PDEDIR)/*.pde)

CFLAGS := $(shell java -cp "$(CLASSPATH)" processing.app.debug.Compiler --dir "$(BASE_DIR)" --board $(CORE) --print-c-flags)
CXXFLAGS := $(shell java -cp "$(CLASSPATH)" processing.app.debug.Compiler  --dir "$(BASE_DIR)" --board $(CORE) --print-cxx-flags)
CLDFLAGS := $(shell java -cp "$(CLASSPATH)" processing.app.debug.Compiler --dir "$(BASE_DIR)" --board $(CORE) --print-ld-flags)

DIRS = CommonTools Midi GUI SDCard MNM MD MidiTools Elektron PageTools MDPages
LDIRS = $(wildcard $(LBASE_DIR)/*) $(CORE_DIR)
INCS = $(foreach dir,$(LDIRS),-I$(dir))

SRCS =  $(foreach dir,$(LDIRS),$(wildcard $(dir)/*.cpp))
#SRCS += $(foreach dir,$(LDIRS),$(wildcard $(dir)/*.c))

OBJS = $(foreach dir,$(LDIRS),$(foreach file,$(wildcard $(dir)/*.cpp),$(subst .cpp,.o,$(file))))
COBJS = $(foreach dir,$(LDIRS),$(foreach file,$(wildcard $(dir)/*.c),$(subst .c,.o,$(file))))
OBJS += $(foreach dir,$(LDIRS),$(foreach file,$(wildcard $(dir)/*.c),$(subst .c,.o,$(file))))
LLVMOBJS = $(foreach obj,$(OBJS),$(subst .o,.llvm.o,$(obj)))

CFLAGS += $(INCS)
CXXFLAGS += $(INCS)
LLVMCFLAGS ?= -emit-llvm -O4 $(CFLAGS)

TARGET ?= $(shell pwd  | sed -e 's/.*\///')

all: $(TARGET).hex

bla: 
	echo "FOO"
	echo "$(CLASSPATH)"
	echo "$(TARGET).cpp"

$(TARGET).cpp: $(PDEFILES) $(wildcard $(CORE_DIR)/*.cxx)
	echo $(wildcard $(CORE_DIR)/*.cxx)
	java -cp "$(CLASSPATH)" processing.app.preproc.PdePreprocessor --dir "$(BASE_DIR)" --board $(CORE) ./$(TARGET) $(PDEFILES)

$(TARGET).o: $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -c $(TARGET).cpp -o $@
	rm $(TARGET).cpp

$(TARGET)whole.elf: $(TARGET).mycpp $(COBJS)
	$(CXX) $(CLDFLAGS) $(CFLAGS) -o $@ -fwhole-program --combine $^ $(SRCS)
	rm -f $(TARGET).mycpp

upload: $(TARGET).hex
	midi-send -I 0x41 -b -i2 -o2 $<

$(TARGET).bc.o: $(TARGET).llvm.o $(LLVMOBJS)
	llvm-link -o $@ -f $^

$(TARGET).elf: $(TARGET).o $(OBJS)
	$(CXX) $(CLDFLAGS) -g -o $@ $^ 
	rm -f $(TARGET).mycpp

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.llvm.o: %.c 
	$(LLVMCC) $(LLVMCFLAGS) -emit-llvm -c $< -o $@

%.llvm.o: %.cpp 
	$(LLVMCXX) $(LLVMCFLAGS) -emit-llvm -c $< -o $@

%.o: %.s 
	$(CC) $(CFLAGS) -c $< -o $@

%.s: %.c
	$(CC) -S $(CFLAGS) -fverbose-asm $< -o $@

%.s: %.cpp
	$(CXX) -S $(CFLAGS) -fverbose-asm $< -o $@

%.os: %.o
	avr-objdump -S $< > $@

%.elfs: %.elf
	avr-objdump -S $< > $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.d:%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@ ; \
	[ -s $@ ] || rm -f $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.ee_srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@


libclean:
	rm -rf $(OBJS)

clean:
	rm -rf *.os *.o *.elf *.elfs *.lst $(TARGET).cpp $(TARGET).hex
