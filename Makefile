PHONE_IP = "192.168.2.105"

.PHONY: build install

install: build copy_pbw
	@echo "INSTALL FANCY WATCH"
	pebble install --phone ${PHONE_IP}

build:
	@echo "BUILD FANCY WATCH"
	pebble build

copy_pbw:
	@echo "COPY PBW FILE TO bin/"
	cp build/pebble-fancywatch.pbw bin/pebble-fancywatch.pbw
