FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  doxygen \
  graphviz \
  telnet \
  netcat \
  tmux \
  clang-format-6.0 \
  && rm -rf /var/lib/apt/lists/*

CMD ["bash", "-l"]
