FROM ubuntu:xenial

ADD . /root

RUN dpkg --add-architecture i386 && apt-get update && apt-get -y install build-essential git

RUN cd /root && make deps_install gtest_install arm_sdk_install

CMD ["/bin/bash"]
