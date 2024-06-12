# Timer overlay
This is a project to display a timer overlayed on top of the rest of your desktop that is controlled with keyboard shortcuts so you can quickly
start a timer any time you need with only a couple key presses

This relies on the [GlobalShortcuts XDG desktop portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html),
which is currently only implemented on KDE Plasma. I am considering reworking it to not rely on the portal, but we'll see if I get around to that



# Dependencies
This project relies on [glew](https://github.com/nigels-com/glew) [glfw3](https://www.glfw.org/) [freetype2](https://freetype.org/) and [sdbus-cpp](https://github.com/Kistler-Group/sdbus-cpp).
You will need to install these libraries to be able to build/run this project

For example, on Arch Linux:
```
sudo pacman -S glew glfw freetype2 sdbus-cpp
```

You will also need to have make and a c++ compiler to build the project
For example, on Arch Linux: (you probably already have these if you are using Arch as they are dependencies of base-devel)
```
sudo pacman -S make gcc
```


# Building
Once the dependencies have been installed, you can compile the project simply with:
```
make
```



# Installing
Installation is likewise very simple, simply run:
```
sudo make install
```

There is also a desktop file provided that you may want to install to /usr/share/applications/, ~/.local/share/applications/, or ~/.config/autostart,
but that is left up to you



# Configuring
In the future, there will be a configuration file to configure the position, size, and font. However, when running under Wayland,
the requested position of the window will not be respected, so to set the position, as well as removing window decorations, and
preventing the window from stealing focus when it is opened, you will need to set window rules. You can see the rules I use below:
![window rules](https://cam123.dev/files/hidden/images/window_rules.png)



# Licensing
The MIT license attached to this project applies to most of the repo, with the exception of one file, ```src/character_utils.cpp```.
This file's core logic was copied from [https://learnopengl.com/In-Practice/Text-Rendering](https://learnopengl.com/In-Practice/Text-Rendering),
and is therefore licensed and copyrighted by its creator, [Joey de Vries](https://twitter.com/JoeyDeVriez) under the CC BY 4.0 license
which you can read about [here](https://creativecommons.org/licenses/by/4.0/) or [here](https://creativecommons.org/licenses/by/4.0/legalcode)

