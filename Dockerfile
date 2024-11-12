FROM devkitpro/devkitppc

COPY --from=ghcr.io/wiiu-env/libcurlwrapper:20240505 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20240426 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20240505 /artifacts $DEVKITPRO

RUN git config --global --add safe.directory /project

WORKDIR /project
