# AssetSystem

Open source, windows based, C++ library for game engines/applications to manage file based assets

Right now the only supported sdk is DirectX11, so make sure you
```
#define ASSET_DIRECTX11
```
before including this library's headers

## Include Into Project
- From src, Copy **asset.h** and **assetLoader.h**, and into the same directory
- ```#include``` assetLoader.h into your project
- Copy the lib file from either debug or release into your project
- Link the lib when compiling