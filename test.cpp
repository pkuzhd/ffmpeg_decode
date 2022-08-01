#include <iostream>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

int main() {
    int fd = open("./pipe", O_WRONLY);
    cout << fd << endl;
    char *data = new char[1920 * 1080 * 3 * 5];
    if (!data)
        cout << "null" << endl;
//    for (int i = 0; i < 1024; ++i)
//        memset(data + i, i, 1024 * 1024);
    auto t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i)
        write(fd, data, 1920 * 1080 * 3 * 5);
    auto t2 = chrono::high_resolution_clock::now();
    cout << chrono::duration<double, milli>(t2 - t1).count() / 1000 << " "
         << 100 / (chrono::duration<double, milli>(t2 - t1).count() / 1000)
         << endl;
    return 0;
}