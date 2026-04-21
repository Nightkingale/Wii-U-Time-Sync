FROM devkitpro/devkitppc

RUN git config --global --add safe.directory /project

WORKDIR /project
