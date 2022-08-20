FROM aflplusplus/aflplusplus

RUN  cd /AFLplusplus/utils/aflpp_driver \
     && make

# docker build . -t adafruit-gps
# docker run -it -v "$PWD:/share" --workdir /share adafruit-gps

# afl-cc -I./src -lm  -lstdc++ harness.cpp src/*.cpp /AFLplusplus/utils/aflpp_driver/aflpp_driver.o /usr/local/lib/afl/afl-compiler-rt.o -o harness
