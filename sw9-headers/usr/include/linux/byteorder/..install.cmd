cmd_dest/include/linux/byteorder/.install := /bin/sh scripts/headers_install.sh dest/include/linux/byteorder ./include/uapi/linux/byteorder big_endian.h little_endian.h; /bin/sh scripts/headers_install.sh dest/include/linux/byteorder ./include/linux/byteorder ; /bin/sh scripts/headers_install.sh dest/include/linux/byteorder ./include/generated/uapi/linux/byteorder ; for F in ; do echo "\#include <asm-generic/$$F>" > dest/include/linux/byteorder/$$F; done; touch dest/include/linux/byteorder/.install