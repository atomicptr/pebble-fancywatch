PHONE_IP="192.168.2.105"

run:
	pebble build
	pebble install --phone ${PHONE_IP}

reinstall:
	pebble install --phone ${PHONE_IP}
