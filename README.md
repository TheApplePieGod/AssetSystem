#AssetSystem

Open source C++ library for game engines/applications to manage file based assets

*Currently hardcoded for Windows with DirectX11 support, with a few std classes*

If you're not including the Windows/DirectX11 or std headers in your project, include
```
#define ASSET_FORWARD_DECLARATIONS
```
before include this library's headers

## Include Into Project
- From src, Copy **asset.h**, **assetLoader.h**, and **shared.h** into the same directory
- ```#include``` assetLoader.h into your project
- Copy the lib file from either debug or release into your project
- Link the lib when compiling