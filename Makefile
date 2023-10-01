
.PHONY: all clean

all:
	@$(MAKE) -C Overlay/
	@mkdir -p Out/switch/.overlays
	@cp Overlay/ReverseNX-RT-ovl.ovl Out/switch/.overlays/ReverseNX-RT-ovl.ovl

clean:
	@$(MAKE) -C Overlay/ clean
	@rm -r -f Out