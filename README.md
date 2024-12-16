# TLCT

The official implementation of the *Tsinghua Lenslet Conversion Tool*.

## Usage

```
Usage: tlct [--help] [--version]
            calib_file

I/O:
            --src VAR --dst VAR

Frame Range:
            [--begin VAR] [--end VAR]

Conversion:
            [--views VAR] [--upsample VAR] [--psizeInflate VAR] [--viewShiftRange VAR] [--patternSize VAR]
            [--psizeShortcutThre VAR]

Positional arguments:
  calib_file           path of the `calib.cfg` [required]

Optional arguments:
  -h, --help           shows help message and exits 
  -v, --version        prints version information and exits 

I/O (detailed usage):
  -i, --src            input yuv420p file [required]
  -o, --dst            output directory [required]

Frame Range (detailed usage):
  -b, --begin          the index of the start frame, left contains, starts from zero [nargs=0..1] [default: 0]
  -e, --end            the index of the end frame, right NOT contains [nargs=0..1] [default: 1]

Conversion (detailed usage):
  --views              viewpoint number [nargs=0..1] [default: 1]
  --upsample           the input image will be upsampled by this scale [nargs=0..1] [default: 1]
  --psizeInflate       the extracted patch will be inflated by this scale [nargs=0..1] [default: 2.15]
  --viewShiftRange     reserve `viewShiftRange*diameter` for view shifting [nargs=0..1] [default: 0.1]
  --patternSize        the size of matching pattern will be `patternSize*diameter` [nargs=0..1] [default: 0.3]
  --psizeShortcutThre  if the difference bit of dhash of MI is smaller than this value, then use the prev. patch size [nargs=0..1] [default: 4]
```

## Recommend Calibration Configs

See [here](https://github.com/lumina37/TLCT-test-data/tree/master/recommend)

## Tested Complier Version

+ gcc v13
+ clang v18
+ msvc v19.29* (*hard to test different version of msvc)
