# FROM devkitpro/devkitppc
FROM ghcr.io/wiiu-env/devkitppc:20250608

COPY --from=ghcr.io/wiiu-env/libcurlwrapper:20240505 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20250204 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20250208 /artifacts $DEVKITPRO

RUN git config --global --add safe.directory /project

WORKDIR /project
