STUDENT=knuth
ASSIGNMENT=2nd-reexam
VERSION=1

TAR_GZ=osm16-2nd-reexam-$(STUDENT)-v$(VERSION).tar.gz

all: $(TAR_GZ)

$(TAR_GZ):
	make -C kudos/kudos/ clean
	make -C kudos/userland/ clean
	tar cvzf $(TAR_GZ) \
	  kudos/kudos/ \
	  kudos/userland/ \
	  kudos/yams.conf

clean:
	rm -rf $(TAR_GZ)

.PHONY: all clean
