CC := gcc
TARGET := day85
SRC := day85.c

CFLAGS := -Wall -Wextra -Werror -g -O0 \
	-fstack-protector-all -fPIE -fcf-protection=none
LDFLAGS := -pie -z noexecstack -Wl,-z,relro,-z,now

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

check: $(TARGET)
	checksec --file=./$(TARGET)

clean:
	rm -f $(TARGET)
