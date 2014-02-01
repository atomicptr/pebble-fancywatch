PHONE_IP = "192.168.2.105"

.PHONY: build install

install: build
	@echo "INSTALL FANCY WATCH"
	pebble install --phone ${PHONE_IP}

build:
	@echo "BUILD FANCY WATCH"
	pebble build