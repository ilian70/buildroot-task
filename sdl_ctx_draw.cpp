#include "SDL_surface.h"
#include "print.h"
#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

bool DirectFramebufferWrite(SDL_Surface *inputSurface, int rgbOrder) //= 0 RGB, 1 BGR
{
  int fb_fd = open("/dev/fb0", O_RDWR);
  if (fb_fd >= 0) 
  {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);

    println("vinfo: xres=", vinfo.xres, " yres=", vinfo.yres,
            " xres_virtual=", vinfo.xres_virtual,
            " yres_virtual=", vinfo.yres_virtual,
            " bits_per_pixel=", vinfo.bits_per_pixel);
    println("finfo: line_length=", finfo.line_length,
            " smem_len=", finfo.smem_len);

    long screensize = vinfo.yres_virtual * finfo.line_length;

    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                             fb_fd, 0);

    SDL_Surface *loadedSurface = inputSurface;
    SDL_Surface *surface_16bpp = nullptr;
    SDL_LockSurface(loadedSurface);

    println("## input surface: w=", loadedSurface->w, " h=", loadedSurface->h,
            " pitch=", loadedSurface->pitch,
            " bpp=", loadedSurface->format->BitsPerPixel);

    if (fbp != MAP_FAILED) 
    {
      int srcBpp = loadedSurface->format->BitsPerPixel;
      int destBpp = vinfo.bits_per_pixel;

      println("## draw direct (mmap) start, srcBpp=", srcBpp,
              " destBpp=", destBpp);

      if (srcBpp != destBpp) {
        SDL_UnlockSurface(loadedSurface);

        surface_16bpp = SDL_ConvertSurfaceFormat(
            loadedSurface, SDL_PIXELFORMAT_RGB565,
            0 // Flags are generally unused and should be 0 in SDL2
        );

        loadedSurface = surface_16bpp;
        SDL_LockSurface(loadedSurface);
      }

      for (int y = 0; y < loadedSurface->h && y < vinfo.yres; y++) {
        long fb_location = y * finfo.line_length;
        long img_location = y * loadedSurface->pitch;

        // copy to FB
        memcpy(fbp + fb_location, (char *)loadedSurface->pixels + img_location,
               std::min((int)loadedSurface->pitch, (int)finfo.line_length));
      }

      munmap(fbp, screensize);

      println("## draw direct (mmap) done ");
    }

    SDL_UnlockSurface(loadedSurface);
    if (surface_16bpp) {
      SDL_FreeSurface(surface_16bpp);
    }

    close(fb_fd); // /dev/fb0

    println("## draw direct all done ");
  }

  return fb_fd >= 0;
}