#HI3518A_NORMAL(rt3070);HI3518A_PT(mt7601);
#HI3518C_MINI_IPC(ar9271);HI3518C_MININVR_IPC(mt7601)

PLATFORM_TYPE =HI3518C_MINI_IPC

#RT3070,AR9271,MT7601
WIFI_CHIP = AR9271

export PLATFORM_TYPE
export WIFI_CHIP

ifeq ($(WIFI_CHIP), )
$(error ERROR:You must define WIFI_CHIP = AR9271 OR RT3070 OR MT7601)
endif

APP_RELEASE_DIR=../AppReleaseDir/



all:
	@echo "     *******************************************************************"
	@echo "            build WIFI_CHIP=$(WIFI_CHIP),support PLATFORM_TYPE=$(PLATFORM_TYPE)"
	@echo "     *******************************************************************"

	make -C src/


clean:
	rm ./src/*.o
	rm ./src/*.a
.PHONY:clean