FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  doxygen \
  graphviz \
  telnet \
  netcat \
  tmux \
  && rm -rf /var/lib/apt/lists/*

CMD ["bash", "-l"]
