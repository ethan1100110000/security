#include <cstdio>

int main() {
    char* buffer = new char[16];
    buffer[0] = 'A';
    std::printf("%c\n", buffer[0]);
    delete[] buffer;
    return 0;
}
