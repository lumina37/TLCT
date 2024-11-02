# TLCT

The official implementation of *Tsinghua Lenslet Conversion Tool*.

## Usage

```shell
tlct /path/to/param.cfg
```

## Configuration File Format

### Generic Options

| option          | type   | description                                                                                  |
|-----------------|--------|----------------------------------------------------------------------------------------------|
| pipeline        | int    | (Optional) The image is captured by Raytrix (0) or TSPC (1), defaults to 0                   |
| Calibration_xml | string | The path of the calibration file                                                             |
| RawImage_Path   | string | The glob pattern of the input images in C-printf style, will be filled with the frame index  |
| Output_Path     | string | The glob pattern of the output images in C-printf style, will be filled with the frame index |
| start_frame     | int    | The index of the start frame, left contains                                                  |
| end_frame       | int    | The index of the end frame, right contains                                                   |
| width           | int    | The pixel width of input image                                                               |
| height          | int    | The pixel height of input image                                                              |

### Fine-tune Options

All fine-tune options are optional

| option                 | type  | description                                                                                    | recommend | default |
|------------------------|-------|------------------------------------------------------------------------------------------------|-----------|---------|
| upsample               | int   | The input image will be upsampled by this scale                                                | 2~4       | 2       |
| psizeInflate           | float | The extracted patch will be inflated by this scale                                             | 1.5~3     | 2.15    |
| maxPsize               | float | Patch size will never be larger than `maxPsize*diameter`                                       | 0.3~1.0   | 0.5     |
| patternSize            | float | The size of matching pattern will be `patternSize*diameter`                                    | 0.25~0.4  | 0.3     |
| psizeShortcutThreshold | int   | If the difference bit of dhash of MI is smaller than this value, then use the prev. patch size | 2~8       | 4       |

### Example of param.cfg

```
pipeline                1
Calibration_xml         Boys.xml
RawImage_Path           Boys/src/Image%03d.png
Output_Path             Boys/dst/frame%03d
start_frame             1
end_frame               300
height                  2048
width                   2048
upsample                2
psizeInflate            2.0
maxPsize                0.5
patternSize             0.3
psizeShortcutThreshold  4
```

### Charset

Must be utf-8

## Compile Options

See `cmake/options.cmake`

## Tested Complier Version

+ gcc v13
+ clang v18
+ msvc v19.39

## Related Document

[(m67487) Multiview Rendering for Tsinghua Single-focused Plenoptic Videos](https://dms.mpeg.expert/doc_end_user/current_document.php?id=92666)
