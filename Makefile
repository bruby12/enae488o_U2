TARGETS = enae488o_u2

all: $(TARGETS)
.PHONY: $(TARGETS)

enae488o_u2: build/triangle.elf build/triangle.hex build/triangle.lss

CC = avr-gcc -std=c99
AVROC = avr-objcopy
AVROD = avr-objdump
AVRUP = avrdude

CFLAGS = -mmcu=atmega328p -Wall -gdwarf-2 -O3 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Ikilolib
CFLAGS += -DF_CPU=8000000

FLASH = -R .eeprom -R .fuse -R .lock -R .signature
EEPROM = -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0  

KILOLIB = kilolib/build/kilolib.a

$(KILOLIB):
	make -C kilolib build/kilolib.a

%.lss: %.elf
	$(AVROD) -d -S $< > $@

%.hex: %.elf
	$(AVROC) -O ihex $(FLASH) $< $@

%.eep: %.elf
	$(AVROC) -O ihex $(EEPROM) $< $@

%.bin: %.elf
	$(AVROC) -O binary $(FLASH) $< $@ 

build:
	mkdir -p $@

build/%.elf: %.c $(KILOLIB)  | build
	$(CC) $(CFLAGS) -o $@ $< $(KILOLIB)

define LABS_TEMPLATE
<div class="container">
$$body$$
</div>
endef
export LABS_TEMPLATE

docs:
	@echo "$$LABS_TEMPLATE" > labs.template
	pandoc --from markdown+fenced_code_blocks --to html --template labs.template README.md --output labs.html
	@rm -f labs.template

clean:
	rm -fR build
	rm -f $(KILOLIB)