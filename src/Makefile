APPLICATION = gps-tracker
RIOTBASE ?= ../RIOT
# this app is device specific
BOARD = lobaro-lorabox
# required riot modules
USEMODULE += fmt
USEMODULE += isrpipe_read_timeout
USEMODULE += xtimer
USEPKG += minmea
# Use by default sx1276 and region EU868
LORA_DRIVER ?= sx1272
LORA_REGION ?= EU868
# Semtech LoRaMAC pkg is required in order to user LoRaWAN
USEPKG += semtech-loramac
# Load the LoRa Module
USEMODULE += $(LORA_DRIVER)

FEATURES_REQUIRED += periph_eeprom

# enable devel helpers
DEVELHELP=0
# Tell LoRaMAC pkg which region should be used
CFLAGS += -DREGION_$(LORA_REGION)
CFLAGS += -DLORAMAC_ACTIVE_REGION=LORAMAC_REGION_$(LORA_REGION)

include $(RIOTBASE)/Makefile.include
