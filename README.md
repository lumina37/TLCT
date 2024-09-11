# TLCT

The official implementation of *Tsinghua Lenslet Conversion Tool*.

## Usage

```shell
RLC40 [param_file_path]
```

## Configuration File Format

### Options

+ `pipeline (int)` The image is captured by Raytrix (0) or TSPC (1).
+ `Calibration_xml (str)` The path of the calibration file.
+ `RawImage_Path (str)` The glob pattern of the input images in C-printf style, filled with the frame index.
+ `Output_Path (str)` The glob pattern of the output images in C-printf style, filled with the frame index.
+ `start_frame (int)` The index of the start frame, left contains.
+ `end_frame (int)` The index of the end frame, right contains.
+ `width (int)` The pixel width of input image.
+ `height (int)` The pixel height of input image.
+ `upsample (int)` The upsample factor.
+ `patternSize (float)` The size of pattern correlated with MI diameter.
+ `gradientBlendingWidth (float)` The size of gradient blending width correlated with MI diameter.
+ `psizeShortcutThreshold (float)` The accept threshold, -1.0 will refuse all, while 0.0 will accept all.

### Example

```
pipeline                1
Calibration_xml         Boys.xml
RawImage_Path           Boys/src/image_%d.png
Output_Path             Boys/dst/%03d
start_frame             1
end_frame               300
height                  2048
width                   2048
upsample                2
patternSize             0.300
gradientBlendingWidth   0.450
psizeShortcutThreshold  -0.9
```

### Charset

Must be utf-8!

## Compile Options

See `cmake/options.cmake`

## Related Document

[(m67487) Multiview Rendering for Tsinghua Single-focused Plenoptic Videos](https://dms.mpeg.expert/doc_end_user/current_document.php?id=92666)
