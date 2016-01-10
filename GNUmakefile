# This is a GNU makefile that uses GNU make extensions

cleaners = $(sort $(wildcard */cleanup.bash))
builders = $(sort $(wildcard */build.bash))


build:
ifneq ($(builders),)
	@for i in $(builders) ; do echo "running: $$i"; $$i ; done
else
	@echo "nothing to build"
endif

clean:
ifneq ($(cleaners),)
	@for i in $(cleaners) ; do echo "running: $$i"; $$i ; done
else
	@echo "nothing to clean"
endif
