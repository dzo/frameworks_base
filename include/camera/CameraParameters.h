/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_CAMERA_PARAMETERS_H
#define ANDROID_HARDWARE_CAMERA_PARAMETERS_H

#include <utils/KeyedVector.h>
#include <utils/String8.h>

namespace android {

struct Size {
    int width;
    int height;

    Size() {
        width = 0;
        height = 0;
    }

    Size(int w, int h) {
        width = w;
        height = h;
    }
};

class CameraParameters
{
public:
    CameraParameters();
    CameraParameters(const String8 &params) { unflatten(params); }
    ~CameraParameters();

    String8 flatten() const;
    void unflatten(const String8 &params);

    void set(const char *key, const char *value);
    void set(const char *key, int value);
    void setFloat(const char *key, float value);
    const char *get(const char *key) const;
    int getInt(const char *key) const;
    float getFloat(const char *key) const;

    void remove(const char *key);

    void setPreviewSize(int width, int height);
    void getPreviewSize(int *width, int *height) const;
    void setPreviewFrameRate(int fps);
    int getPreviewFrameRate() const;
    void setPreviewFrameRateMode(const char *mode);
    const char *getPreviewFrameRateMode() const;
    void setPreviewFormat(const char *format);
    const char *getPreviewFormat() const;
    void setPictureSize(int width, int height);
    void getPictureSize(int *width, int *height) const;
    void setPictureFormat(const char *format);
    const char *getPictureFormat() const;
    void setTouchIndexAec(int x, int y);
    void getTouchIndexAec(int *x, int *y) const;

    enum {
        CAMERA_ORIENTATION_UNKNOWN = 0,
        CAMERA_ORIENTATION_PORTRAIT = 1,
        CAMERA_ORIENTATION_LANDSCAPE = 2,
    };
    int getOrientation() const;
    void setOrientation(int orientation);

    void dump() const;
    status_t dump(int fd, const Vector<String16>& args) const;

    // Parameter keys to communicate between camera application and driver.
    // The access (read/write, read only, or write only) is viewed from the
    // perspective of applications, not driver.

    // Preview frame size in pixels (width x height).
    // Example value: "480x320". Read/Write.
    static const char KEY_PREVIEW_SIZE[];
    // Supported preview frame sizes in pixels.
    // Example value: "800x600,480x320". Read only.
    static const char KEY_SUPPORTED_PREVIEW_SIZES[];
    // The image format for preview frames.
    // Example value: "yuv420sp" or PIXEL_FORMAT_XXX constants. Read/write.
    static const char KEY_PREVIEW_FORMAT[];
    // Supported image formats for preview frames.
    // Example value: "yuv420sp,yuv422i-yuyv". Read only.
    static const char KEY_SUPPORTED_PREVIEW_FORMATS[];
    // Number of preview frames per second.
    // Example value: "15". Read/write.
    static const char KEY_PREVIEW_FRAME_RATE[];
    // Supported number of preview frames per second.
    // Example value: "24,15,10". Read.
    static const char KEY_SUPPORTED_PREVIEW_FRAME_RATES[];
    // The mode of preview frame rate.
    // Example value: "frame-rate-auto, frame-rate-fixed".
    static const char KEY_PREVIEW_FRAME_RATE_MODE[];
    static const char KEY_SUPPORTED_PREVIEW_FRAME_RATE_MODES[];
    static const char KEY_PREVIEW_FRAME_RATE_AUTO_MODE[];
    static const char KEY_PREVIEW_FRAME_RATE_FIXED_MODE[];
    // The dimensions for captured pictures in pixels (width x height).
    // Example value: "1024x768". Read/write.
    static const char KEY_PICTURE_SIZE[];
    // Supported dimensions for captured pictures in pixels.
    // Example value: "2048x1536,1024x768". Read only.
    static const char KEY_SUPPORTED_PICTURE_SIZES[];
    // The image format for captured pictures.
    // Example value: "jpeg" or PIXEL_FORMAT_XXX constants. Read/write.
    static const char KEY_PICTURE_FORMAT[];
    // Supported image formats for captured pictures.
    // Example value: "jpeg,rgb565". Read only.
    static const char KEY_SUPPORTED_PICTURE_FORMATS[];
    // The width (in pixels) of EXIF thumbnail in Jpeg picture.
    // Example value: "512". Read/write.
    static const char KEY_JPEG_THUMBNAIL_WIDTH[];
    // The height (in pixels) of EXIF thumbnail in Jpeg picture.
    // Example value: "384". Read/write.
    static const char KEY_JPEG_THUMBNAIL_HEIGHT[];

    //++TODO is the following parameter is needed when jpeg thumbnail is available
    static const char KEY_SUPPORTED_THUMBNAIL_SIZES[];

    // Supported EXIF thumbnail sizes (width x height). 0x0 means not thumbnail
    // in EXIF.
    // Example value: "512x384,320x240,0x0". Read only.
    static const char KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES[];
    // The quality of the EXIF thumbnail in Jpeg picture. The range is 1 to 100,
    // with 100 being the best.
    // Example value: "90". Read/write.
    static const char KEY_JPEG_THUMBNAIL_QUALITY[];
    // Jpeg quality of captured picture. The range is 1 to 100, with 100 being
    // the best.
    // Example value: "90". Read/write.
    static const char KEY_JPEG_QUALITY[];
    // The orientation of the device in degrees. For example, suppose the
    // natural position of the device is landscape. If the user takes a picture
    // in landscape mode in 2048x1536 resolution, the rotation will be set to
    // "0". If the user rotates the phone 90 degrees clockwise, the rotation
    // should be set to "90".
    // The camera driver can set orientation in the EXIF header without rotating
    // the picture. Or the driver can rotate the picture and the EXIF thumbnail.
    // If the Jpeg picture is rotated, the orientation in the EXIF header should
    // be missing or 1 (row #0 is top and column #0 is left side). The driver
    // should not set default value for this parameter.
    // Example value: "0" or "90" or "180" or "270". Write only.
    static const char KEY_ROTATION[];
    // GPS latitude coordinate. This will be stored in JPEG EXIF header.
    // Example value: "25.032146". Write only.
    static const char KEY_GPS_LATITUDE[];
    // GPS longitude coordinate. This will be stored in JPEG EXIF header.
    // Example value: "121.564448". Write only.
    static const char KEY_GPS_LONGITUDE[];
    // GPS altitude. This will be stored in JPEG EXIF header.
    // Example value: "21.0". Write only.
    static const char KEY_GPS_ALTITUDE[];

    static const char KEY_GPS_LATITUDE_REF[];
    static const char KEY_GPS_LONGITUDE_REF[];
    static const char KEY_GPS_ALTITUDE_REF[];
    static const char KEY_GPS_STATUS[];
    static const char KEY_EXIF_DATETIME[];

    static const char KEY_AUTO_EXPOSURE[];
    static const char KEY_SUPPORTED_AUTO_EXPOSURE[];
    static const char KEY_ISO_MODE[];
    static const char KEY_SUPPORTED_ISO_MODES[];
    static const char KEY_LENSSHADE[] ;
    static const char KEY_SUPPORTED_LENSSHADE_MODES[] ;
    static const char KEY_SHARPNESS[];
    static const char KEY_MAX_SHARPNESS[];
    static const char KEY_CONTRAST[];
    static const char KEY_MAX_CONTRAST[];
    static const char KEY_SATURATION[];
    static const char KEY_MAX_SATURATION[];

    // Values for auto exposure settings.
    static const char AUTO_EXPOSURE_FRAME_AVG[];
    static const char AUTO_EXPOSURE_CENTER_WEIGHTED[];
    static const char AUTO_EXPOSURE_SPOT_METERING[];



    // GPS timestamp (UTC in seconds since January 1, 1970). This should be
    // stored in JPEG EXIF header.
    // Example value: "1251192757". Write only.
    static const char KEY_GPS_TIMESTAMP[];
    // GPS Processing Method
    // Example value: "GPS" or "NETWORK". Write only.
    static const char KEY_GPS_PROCESSING_METHOD[];
    // Current white balance setting.
    // Example value: "auto" or WHITE_BALANCE_XXX constants. Read/write.
    static const char KEY_WHITE_BALANCE[];
    // Supported white balance settings.
    // Example value: "auto,incandescent,daylight". Read only.
    static const char KEY_SUPPORTED_WHITE_BALANCE[];
    // Current color effect setting.
    // Example value: "none" or EFFECT_XXX constants. Read/write.
    static const char KEY_EFFECT[];
    // Supported color effect settings.
    // Example value: "none,mono,sepia". Read only.
    static const char KEY_SUPPORTED_EFFECTS[];
    //Touch Af/AEC settings.
    static const char KEY_TOUCH_AF_AEC[];
    static const char KEY_SUPPORTED_TOUCH_AF_AEC[];
    //Touch Index for AEC.
    static const char KEY_TOUCH_INDEX_AEC[];
    // Current antibanding setting.
    // Example value: "auto" or ANTIBANDING_XXX constants. Read/write.
    static const char KEY_ANTIBANDING[];
    // Supported antibanding settings.
    // Example value: "auto,50hz,60hz,off". Read only.
    static const char KEY_SUPPORTED_ANTIBANDING[];
    // Current scene mode.
    // Example value: "auto" or SCENE_MODE_XXX constants. Read/write.
    static const char KEY_SCENE_MODE[];
    // Supported scene mode settings.
    // Example value: "auto,night,fireworks". Read only.
    static const char KEY_SUPPORTED_SCENE_MODES[];
    // Current flash mode.
    // Example value: "auto" or FLASH_MODE_XXX constants. Read/write.
    static const char KEY_FLASH_MODE[];
    // Supported flash modes.
    // Example value: "auto,on,off". Read only.
    static const char KEY_SUPPORTED_FLASH_MODES[];
    // Current focus mode. If the camera does not support auto-focus, the value
    // should be FOCUS_MODE_FIXED. If the focus mode is not FOCUS_MODE_FIXED or
    // or FOCUS_MODE_INFINITY, applications should call
    // CameraHardwareInterface.autoFocus to start the focus.
    // Example value: "auto" or FOCUS_MODE_XXX constants. Read/write.
    static const char KEY_FOCUS_MODE[];
    // Supported focus modes.
    // Example value: "auto,macro,fixed". Read only.
    static const char KEY_SUPPORTED_FOCUS_MODES[];
    // Focal length in millimeter.
    // Example value: "4.31". Read only.
    static const char KEY_FOCAL_LENGTH[];
    // Horizontal angle of view in degrees.
    // Example value: "54.8". Read only.
    static const char KEY_HORIZONTAL_VIEW_ANGLE[];
    // Vertical angle of view in degrees.
    // Example value: "42.5". Read only.
    static const char KEY_VERTICAL_VIEW_ANGLE[];
    // Exposure compensation index. 0 means exposure is not adjusted.
    // Example value: "0" or "5". Read/write.
    static const char KEY_EXPOSURE_COMPENSATION[];
    // The maximum exposure compensation index (>=0).
    // Example value: "6". Read only.
    static const char KEY_MAX_EXPOSURE_COMPENSATION[];
    // The minimum exposure compensation index (<=0).
    // Example value: "-6". Read only.
    static const char KEY_MIN_EXPOSURE_COMPENSATION[];
    // The exposure compensation step. Exposure compensation index multiply by
    // step eqals to EV. Ex: if exposure compensation index is 6 and step is
    // 0.3333, EV is -2.
    // Example value: "0.333333333" or "0.5". Read only.
    static const char KEY_EXPOSURE_COMPENSATION_STEP[];
    // Current zoom value.
    // Example value: "0" or "6". Read/write.
    static const char KEY_ZOOM[];
    // Maximum zoom value.
    // Example value: "6". Read only.
    static const char KEY_MAX_ZOOM[];
    // The zoom ratios of all zoom values. The zoom ratio is in 1/100
    // increments. Ex: a zoom of 3.2x is returned as 320. The number of list
    // elements is KEY_MAX_ZOOM + 1. The first element is always 100. The last
    // element is the zoom ratio of zoom value KEY_MAX_ZOOM.
    // Example value: "100,150,200,250,300,350,400". Read only.
    static const char KEY_ZOOM_RATIOS[];
    // Whether zoom is supported. Zoom is supported if the value is "true". Zoom
    // is not supported if the value is not "true" or the key does not exist.
    // Example value: "true". Read only.
    static const char KEY_ZOOM_SUPPORTED[];
    // Whether if smooth zoom is supported. Smooth zoom is supported if the
    // value is "true". It is not supported if the value is not "true" or the
    // key does not exist.
    // See CAMERA_CMD_START_SMOOTH_ZOOM, CAMERA_CMD_STOP_SMOOTH_ZOOM, and
    // CAMERA_MSG_ZOOM in frameworks/base/include/camera/Camera.h.
    // Example value: "true". Read only.
    static const char KEY_SMOOTH_ZOOM_SUPPORTED[];

    // Value for KEY_ZOOM_SUPPORTED or KEY_SMOOTH_ZOOM_SUPPORTED.
    static const char TRUE[];

    //Continuous AF.
    static const char KEY_CONTINUOUS_AF[];
    static const char KEY_SUPPORTED_CONTINUOUS_AF[];

    //Continuous AF.
    static const char KEY_CAF[];
    static const char KEY_SUPPORTED_CAF[];

    // Values for white balance settings.
    static const char WHITE_BALANCE_AUTO[];
    static const char WHITE_BALANCE_INCANDESCENT[];
    static const char WHITE_BALANCE_FLUORESCENT[];
    static const char WHITE_BALANCE_WARM_FLUORESCENT[];
    static const char WHITE_BALANCE_DAYLIGHT[];
    static const char WHITE_BALANCE_CLOUDY_DAYLIGHT[];
    static const char WHITE_BALANCE_TWILIGHT[];
    static const char WHITE_BALANCE_SHADE[];

    // Values for effect settings.
    static const char EFFECT_NONE[];
    static const char EFFECT_MONO[];
    static const char EFFECT_NEGATIVE[];
    static const char EFFECT_SOLARIZE[];
    static const char EFFECT_SEPIA[];
    static const char EFFECT_POSTERIZE[];
    static const char EFFECT_WHITEBOARD[];
    static const char EFFECT_BLACKBOARD[];
    static const char EFFECT_AQUA[];

    // Values for Touch AF/AEC
    static const char TOUCH_AF_AEC_OFF[] ;
    static const char TOUCH_AF_AEC_ON[] ;

    // Values for antibanding settings.
    static const char ANTIBANDING_AUTO[];
    static const char ANTIBANDING_50HZ[];
    static const char ANTIBANDING_60HZ[];
    static const char ANTIBANDING_OFF[];

    // Values for flash mode settings.
    // Flash will not be fired.
    static const char FLASH_MODE_OFF[];
    // Flash will be fired automatically when required. The flash may be fired
    // during preview, auto-focus, or snapshot depending on the driver.
    static const char FLASH_MODE_AUTO[];
    // Flash will always be fired during snapshot. The flash may also be
    // fired during preview or auto-focus depending on the driver.
    static const char FLASH_MODE_ON[];
    // Flash will be fired in red-eye reduction mode.
    static const char FLASH_MODE_RED_EYE[];
    // Constant emission of light during preview, auto-focus and snapshot.
    // This can also be used for video recording.
    static const char FLASH_MODE_TORCH[];

    // Values for scene mode settings.
    static const char SCENE_MODE_AUTO[];
    static const char SCENE_MODE_ACTION[];
    static const char SCENE_MODE_PORTRAIT[];
    static const char SCENE_MODE_LANDSCAPE[];
    static const char SCENE_MODE_NIGHT[];
    static const char SCENE_MODE_NIGHT_PORTRAIT[];
    static const char SCENE_MODE_THEATRE[];
    static const char SCENE_MODE_BEACH[];
    static const char SCENE_MODE_SNOW[];
    static const char SCENE_MODE_SUNSET[];
    static const char SCENE_MODE_STEADYPHOTO[];
    static const char SCENE_MODE_FIREWORKS[];
    static const char SCENE_MODE_SPORTS[];
    static const char SCENE_MODE_PARTY[];
    static const char SCENE_MODE_CANDLELIGHT[];
    static const char SCENE_MODE_BACKLIGHT[];
    static const char SCENE_MODE_FLOWERS[];
    // Applications are looking for a barcode. Camera driver will be optimized
    // for barcode reading.
    static const char SCENE_MODE_BARCODE[];

    // Formats for setPreviewFormat and setPictureFormat.
    static const char PIXEL_FORMAT_YUV422SP[];
    static const char PIXEL_FORMAT_YUV420SP[]; // NV21
    static const char PIXEL_FORMAT_YUV420SP_ADRENO[]; // ADRENO
    static const char PIXEL_FORMAT_YUV422I[]; // YUY2
    static const char PIXEL_FORMAT_RGB565[];
    static const char PIXEL_FORMAT_JPEG[];
    static const char PIXEL_FORMAT_RAW[];

    // Values for focus mode settings.
    // Auto-focus mode.
    static const char FOCUS_MODE_AUTO[];
    // Focus is set at infinity. Applications should not call
    // CameraHardwareInterface.autoFocus in this mode.
    static const char FOCUS_MODE_INFINITY[];
    static const char FOCUS_MODE_NORMAL[];
    static const char FOCUS_MODE_MACRO[];
    // Focus is fixed. The camera is always in this mode if the focus is not
    // adjustable. If the camera has auto-focus, this mode can fix the
    // focus, which is usually at hyperfocal distance. Applications should
    // not call CameraHardwareInterface.autoFocus in this mode.
    static const char FOCUS_MODE_FIXED[];
    // Extended depth of field (EDOF). Focusing is done digitally and
    // continuously. Applications should not call
    // CameraHardwareInterface.autoFocus in this mode.
    static const char FOCUS_MODE_EDOF[];

    static const char ISO_AUTO[];
    static const char ISO_HJR[] ;
    static const char ISO_100[];
    static const char ISO_200[] ;
    static const char ISO_400[];
    static const char ISO_800[];
    static const char ISO_1600[];
    // Values for Lens Shading
    static const char LENSSHADE_ENABLE[] ;
    static const char LENSSHADE_DISABLE[] ;

    // Values for Continuous AF
    static const char CONTINUOUS_AF_OFF[] ;
    static const char CONTINUOUS_AF_ON[] ;

    // Values for Continuous AF
    static const char CAF_OFF[] ;
    static const char CAF_ON[] ;

private:
    DefaultKeyedVector<String8,String8>    mMap;
};

}; // namespace android

#endif

