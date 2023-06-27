FROM ghcr.io/wiiu-env/devkitppc:20230616

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20230215 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20230423 /artifacts $DEVKITPRO

WORKDIR project