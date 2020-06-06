
.PHONY: all clean

all:
	@$(MAKE) -C Plugin/
	@$(MAKE) -C Overlay/
	@mkdir -p Out/SaltySD/plugins/
	@mkdir -p Out/switch/.overlays
	@cp Overlay/ReverseNX-RT-ovl.ovl Out/switch/.overlays/ReverseNX-RT-ovl.ovl
	@cp Plugin/ReverseNX-RT.elf Out/SaltySD/plugins/ReverseNX-RT.elf

clean:
	@$(MAKE) -C Plugin/ clean
	@$(MAKE) -C Overlay/ clean
	@rm -r -f Out