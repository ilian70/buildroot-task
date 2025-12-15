How to add redis-image-viewer to Buildroot (2025.11.x):

1) Copy the package directory into Buildroot:
   cp -a buildroot-package/redis-image-viewer ~/Projects/buildroot/package/

2) Add to the package menu (e.g., near other user packages):
   edit ~/Projects/buildroot/package/Config.in and add:
       source "package/redis-image-viewer/Config.in"

3) Configure Buildroot:
   make menuconfig
     -> Target packages
       -> Libraries
         -> Graphics
           [*] sdl2 (auto selected)
           [*] sdl2_image (auto selected)
       -> Networking applications
           [*] redis-image-viewer

    Note: package requires BR2_INIT_SYSTEMD. Make sure Init system is set to systemd.

4) Build:
   make redis-image-viewer-dirclean   # if rebuilding
   make

Notes:
    /usr/bin/redis_image_viewer
    /etc/redis-image-viewer/config.json (from app.cfg.json)
    /var/lib/redis-image-viewer/images/ (created, sample PNGs if present in source images/)

Systemd service
- The package installs /usr/lib/systemd/system/redis-image-viewer.service and
   enables it via multi-user.target.wants so it starts on boot.
- Service runs: /usr/bin/redis_image_viewer --config /etc/redis-image-viewer/config.json
