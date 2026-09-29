# COSC 407 / 507 -- one toolchain for every lab this term.
#
# The point of this image is that nobody spends lab time on installation, and
# that every measurement in the course is taken on a comparable toolchain. It is
# used two ways, from the same definition:
#
#   * GitHub Codespaces -- nothing to install, runs in a browser
#   * Docker or Podman on your own machine -- for working offline
#
# Built and published by the course, not by students. See labs/_devcontainer/README.md.

FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# build-essential  gcc, make, libc headers
# clang            a second opinion on your code, and ThreadSanitizer
# gdb, valgrind    debugging, and Helgrind for race detection
# libomp-dev       OpenMP runtime, needed from Lab 4 onward
# hwloc            lstopo, so you can report what machine you measured on
# python3 + matplotlib  plotting scaling curves from Lab 4 onward
# libcriterion-dev unit testing in C -- the JUnit of this course
# gcovr            coverage over gcov -- the JaCoCo of this course
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      clang \
      gdb \
      valgrind \
      libomp-dev \
      hwloc \
      python3 \
      python3-matplotlib \
      libcriterion-dev \
      gcovr \
      curl \
      ca-certificates \
      less \
      procps \
      time \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Mull: mutation testing, the PIT of this course. It is not in the Ubuntu
# archive; upstream publishes a Cloudsmith apt repository, and the package is
# named for the LLVM it was built against. Ubuntu 24.04 ships clang-18, so it
# must be mull-18: mull-19 installs cleanly and then fails to load its pass
# plugin into clang-18. Mutation testing only earns its keep when automated,
# which is why it is installed here
# rather than asked of students by hand.
#   https://mull.readthedocs.io/en/latest/Installation.html
RUN curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.deb.sh' \
      | bash \
 && apt-get update \
 && apt-get install -y --no-install-recommends mull-18 \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Make the core count obvious the moment a shell opens: every lab asks you to
# report it, and a timing without it is not a measurement.
RUN printf '%s\n' \
    'echo "COSC 407 lab container. $(nproc) cores visible. gcc $(gcc -dumpversion)."' \
    >> /etc/bash.bashrc

WORKDIR /workspaces
