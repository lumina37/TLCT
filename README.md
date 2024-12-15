# TLCT

The official implementation of *Tsinghua Lenslet Conversion Tool*.

## Usage

```
Usage: tlct [--help] [--version]
            calib_file

I/O:
            --src VAR --dst VAR

Frame Range:
            [--begin VAR] [--end VAR]

Camera Specification:
            [--multiFocus] [--isKepler]

Conversion:
            [--views VAR] [--upsample VAR] [--psizeInflate VAR] [--maxPsize VAR] [--patternSize VAR]
            [--psizeShortcutThre VAR]

Positional arguments:
  calib_file           Path of the `calibration.toml` [required]

Optional arguments:
  -h, --help           shows help message and exits 
  -v, --version        prints version information and exits 

I/O (detailed usage):
  -i, --src            Input yuv420 planar file [required]
  -o, --dst            Output directory, and the output file name is like 'v000-1920x1080.yuv' (v{view}-{wdt}x{hgt}.yuv) [required]

Frame Range (detailed usage):
  -b, --begin          The index of the start frame, left contains, starts from zero [nargs=0..1] [default: 0]
  -e, --end            The index of the end frame, right NOT contains [nargs=0..1] [default: 1]

Camera Specification (detailed usage):
  --multiFocus         Is MFPC 
  --isKepler           Is the main image real 

Conversion (detailed usage):
  --views              Viewpoint number [nargs=0..1] [default: 1]
  --upsample           The input image will be upsampled by this scale [nargs=0..1] [default: 1]
  --psizeInflate       The extracted patch will be inflated by this scale [nargs=0..1] [default: 2.15]
  --maxPsize           Patch size will never be larger than `maxPsize*diameter` [nargs=0..1] [default: 0.5]
  --patternSize        The size of matching pattern will be `patternSize*diameter` [nargs=0..1] [default: 0.3]
  --psizeShortcutThre  If the difference bit of dhash of MI is smaller than this value, then use the prev. patch size [nargs=0..1] [default: 4]
```

## Official Calibration Tomls

See [here](https://github.com/lumina37/TLCT-test-data/tree/master/recommend)

## Tested Complier Version

+ gcc v13
+ clang v18
+ msvc v19.29* (*hard to test different version of msvc)
