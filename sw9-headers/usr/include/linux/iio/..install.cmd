cmd_dest/include/linux/iio/.install := /bin/sh scripts/headers_install.sh dest/include/linux/iio ./include/uapi/linux/iio events.h types.h; /bin/sh scripts/headers_install.sh dest/include/linux/iio ./include/linux/iio ; /bin/sh scripts/headers_install.sh dest/include/linux/iio ./include/generated/uapi/linux/iio ; for F in ; do echo "\#include <asm-generic/$$F>" > dest/include/linux/iio/$$F; done; touch dest/include/linux/iio/.install
