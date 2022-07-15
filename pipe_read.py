import os

fd = os.open("/home/zhd/pipe", os.O_RDONLY)
while True:
    s = os.read(fd, 1024 * 1024)
    print(len(s))
    break

