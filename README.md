# Mallard 64 Scene

This simply is a foray into learning and experimenting with N64 Homebrew using Libdragon and Tiny3D. All work is building off of SpookyIluha's [BrewChristmas](https://github.com/SpookyIluha/BrewChristmas) example. Cheers to the N64Brew community!

## Build with [libdragon-docker](https://github.com/anacierdem/libdragon-docker)

### Clone
Clone this repository with `--recurse-submodules`:
```bash
git clone https://github.com/joshkautz/Mallard64Scene.git --recurse-submodules
```
If you've already cloned the repository without the `--recurse-submodules` flag, run:
```bash
git submodule init
git submodule update
```

### Initialize

Initialize Docker container:
```bash
libdragon init
```

Install Tiny3D on the Docker container.

```bash
libdragon make -C tiny3d install
libdragon make -C tiny3d/tools/gltf_importer
libdragon make -C tiny3d/tools/gltf_importer install
```

## Build

```bash
libdragon make
```

## Screenshots

![brew_christmas 2024-12-28 15-49-07](https://github.com/user-attachments/assets/c8269d4e-f95d-4704-90a3-5b32eed63a72)
![image](https://github.com/user-attachments/assets/2386fea4-b8cb-4163-b870-907d58285a21)
![image](https://github.com/user-attachments/assets/6f803b69-6307-4ea9-82d5-fd7b5b8bc3e8)
![brew_christmas 2024-12-28 15-56-41](https://github.com/user-attachments/assets/b2149cd5-d49a-4ce9-9d8e-afded3529a73)
![brew_christmas 2024-12-28 15-56-36](https://github.com/user-attachments/assets/0f127e4c-cb30-4c6b-8caf-ffdeefd63d00)
