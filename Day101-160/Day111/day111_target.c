#include <stdio.h>
#include <unistd.h>

int main(void)
{
    unsigned char buf[4] = {0};
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));

    if (n < 1)
        return 0;

    if (buf[0] == 'F') {
        if (n > 1 && buf[1] == 'U') {
            if (n > 2 && buf[2] == 'Z') {
                if (n > 3 && buf[3] == 'Z')
                    puts("deep path");
            }
        }
    }

    return 0;
}
