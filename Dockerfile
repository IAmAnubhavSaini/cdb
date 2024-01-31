FROM ubuntu:latest

RUN DEBIAN_FRONTEND=noninteractive apt update && DEBIAN_FRONTEND=noninteractive apt upgrade --yes 
RUN DEBIAN_FRONTEND=noninteractive apt install --yes build-essential wget  libssl-dev libcurl4-openssl-dev


RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.28.2/cmake-3.28.2.tar.gz -O /tmp/cmake.tar.gz 
RUN tar -xzf /tmp/cmake.tar.gz -C /tmp/
RUN mv /tmp/cmake-* /usr/local/cmake
RUN cd /usr/local/cmake && ./bootstrap && make && make install

# RUN echo `whereis cmake`
# RUN ls -lahR /usr/local/cmake

WORKDIR /app

COPY . .

RUN cmake .
RUN make

RUN echo "cd /app"
RUN cd /app
RUN echo "pwd"
RUN pwd
# RUN ls -lahR /app

## Now run it:
## docker run --rm -it -v `pwd`:/app cdb-cmake-build:latest
## # cmake .
## # make
## # ./build_db sample-db
## # > insert 10 "your-user" "your-email" "your-password"
## # > select
## # > .exit

