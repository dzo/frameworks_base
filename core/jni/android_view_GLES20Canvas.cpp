/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "OpenGLRenderer"

#include <EGL/egl.h>

#include "jni.h"
#include "GraphicsJNI.h"
#include <nativehelper/JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <utils/ResourceTypes.h>

#include <gui/SurfaceTexture.h>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkMatrix.h>
#include <SkPaint.h>
#include <SkRegion.h>
#include <SkScalerContext.h>
#include <SkTemplates.h>
#include <SkXfermode.h>

#include <DisplayListRenderer.h>
#include <LayerRenderer.h>
#include <OpenGLRenderer.h>
#include <SkiaShader.h>
#include <SkiaColorFilter.h>
#include <Rect.h>

#include <TextLayout.h>

namespace android {

using namespace uirenderer;

/**
 * Note: OpenGLRenderer JNI layer is generated and compiled only on supported
 *       devices. This means all the logic must be compiled only when the
 *       preprocessor variable USE_OPENGL_RENDERER is defined.
 */
#ifdef USE_OPENGL_RENDERER

///////////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////////

// Debug
#define DEBUG_RENDERER 0

// Debug
#if DEBUG_RENDERER
    #define RENDERER_LOGD(...) LOGD(__VA_ARGS__)
#else
    #define RENDERER_LOGD(...)
#endif

#define MODIFIER_SHADOW 1
#define MODIFIER_SHADER 2
#define MODIFIER_COLOR_FILTER 4

// ----------------------------------------------------------------------------

static struct {
    jmethodID set;
} gRectClassInfo;

// ----------------------------------------------------------------------------
// Misc
// ----------------------------------------------------------------------------

static jboolean android_view_GLES20Canvas_preserveBackBuffer(JNIEnv* env, jobject clazz) {
    EGLDisplay display = eglGetCurrentDisplay();
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);

    eglGetError();
    eglSurfaceAttrib(display, surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);

    EGLint error = eglGetError();
    RENDERER_LOGD("Could not enable buffer preserved swap behavior (%x)", error);

    return error == EGL_SUCCESS;
}

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------

static OpenGLRenderer* android_view_GLES20Canvas_createRenderer(JNIEnv* env, jobject clazz) {
    RENDERER_LOGD("Create OpenGLRenderer");
    return new OpenGLRenderer;
}

static void android_view_GLES20Canvas_destroyRenderer(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    RENDERER_LOGD("Destroy OpenGLRenderer");
    delete renderer;
}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

static void android_view_GLES20Canvas_setViewport(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint width, jint height) {
    renderer->setViewport(width, height);
}

static void android_view_GLES20Canvas_prepare(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jboolean opaque) {
    renderer->prepare(opaque);
}

static void android_view_GLES20Canvas_prepareDirty(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint left, jint top, jint right, jint bottom,
        jboolean opaque) {
    renderer->prepareDirty(left, top, right, bottom, opaque);
}

static void android_view_GLES20Canvas_finish(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    renderer->finish();
}

static bool android_view_GLES20Canvas_callDrawGLFunction(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, Functor *functor) {
    android::uirenderer::Rect dirty;
    return renderer->callDrawGLFunction(functor, dirty);
}

// ----------------------------------------------------------------------------
// State
// ----------------------------------------------------------------------------

static jint android_view_GLES20Canvas_save(JNIEnv* env, jobject clazz, OpenGLRenderer* renderer,
        jint flags) {
    return renderer->save(flags);
}

static jint android_view_GLES20Canvas_getSaveCount(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    return renderer->getSaveCount();
}

static void android_view_GLES20Canvas_restore(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    renderer->restore();
}

static void android_view_GLES20Canvas_restoreToCount(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint saveCount) {
    renderer->restoreToCount(saveCount);
}

// ----------------------------------------------------------------------------
// Layers
// ----------------------------------------------------------------------------

static jint android_view_GLES20Canvas_saveLayer(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        SkPaint* paint, jint saveFlags) {
    return renderer->saveLayer(left, top, right, bottom, paint, saveFlags);
}

static jint android_view_GLES20Canvas_saveLayerAlpha(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        jint alpha, jint saveFlags) {
    return renderer->saveLayerAlpha(left, top, right, bottom, alpha, saveFlags);
}

// ----------------------------------------------------------------------------
// Clipping
// ----------------------------------------------------------------------------

static bool android_view_GLES20Canvas_quickReject(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        SkCanvas::EdgeType edge) {
    return renderer->quickReject(left, top, right, bottom);
}

static bool android_view_GLES20Canvas_clipRectF(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        SkRegion::Op op) {
    return renderer->clipRect(left, top, right, bottom, op);
}

static bool android_view_GLES20Canvas_clipRect(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint left, jint top, jint right, jint bottom,
        SkRegion::Op op) {
    return renderer->clipRect(float(left), float(top), float(right), float(bottom), op);
}

static bool android_view_GLES20Canvas_getClipBounds(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jobject rect) {
    const android::uirenderer::Rect& bounds(renderer->getClipBounds());

    env->CallVoidMethod(rect, gRectClassInfo.set,
            int(bounds.left), int(bounds.top), int(bounds.right), int(bounds.bottom));

    return !bounds.isEmpty();
}

// ----------------------------------------------------------------------------
// Transforms
// ----------------------------------------------------------------------------

static void android_view_GLES20Canvas_translate(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat dx, jfloat dy) {
    renderer->translate(dx, dy);
}

static void android_view_GLES20Canvas_rotate(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat degrees) {
    renderer->rotate(degrees);
}

static void android_view_GLES20Canvas_scale(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat sx, jfloat sy) {
    renderer->scale(sx, sy);
}

static void android_view_GLES20Canvas_skew(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat sx, jfloat sy) {
    renderer->skew(sx, sy);
}

static void android_view_GLES20Canvas_setMatrix(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkMatrix* matrix) {
    renderer->setMatrix(matrix);
}

static const float* android_view_GLES20Canvas_getNativeMatrix(JNIEnv* env,
        jobject clazz, OpenGLRenderer* renderer) {
    return renderer->getMatrix();
}

static void android_view_GLES20Canvas_getMatrix(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkMatrix* matrix) {
    renderer->getMatrix(matrix);
}

static void android_view_GLES20Canvas_concatMatrix(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkMatrix* matrix) {
    renderer->concatMatrix(matrix);
}

// ----------------------------------------------------------------------------
// Drawing
// ----------------------------------------------------------------------------

static void android_view_GLES20Canvas_drawBitmap(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkBitmap* bitmap, jbyteArray buffer, float left,
        float top, SkPaint* paint) {
    // This object allows the renderer to allocate a global JNI ref to the buffer object.
    JavaHeapBitmapRef bitmapRef(env, bitmap, buffer);

    renderer->drawBitmap(bitmap, left, top, paint);
}

static void android_view_GLES20Canvas_drawBitmapRect(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkBitmap* bitmap, jbyteArray buffer,
        float srcLeft, float srcTop, float srcRight, float srcBottom,
        float dstLeft, float dstTop, float dstRight, float dstBottom, SkPaint* paint) {
    // This object allows the renderer to allocate a global JNI ref to the buffer object.
    JavaHeapBitmapRef bitmapRef(env, bitmap, buffer);

    renderer->drawBitmap(bitmap, srcLeft, srcTop, srcRight, srcBottom,
            dstLeft, dstTop, dstRight, dstBottom, paint);
}

static void android_view_GLES20Canvas_drawBitmapMatrix(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkBitmap* bitmap, jbyteArray buffer, SkMatrix* matrix,
        SkPaint* paint) {
    // This object allows the renderer to allocate a global JNI ref to the buffer object.
    JavaHeapBitmapRef bitmapRef(env, bitmap, buffer);

    renderer->drawBitmap(bitmap, matrix, paint);
}

static void android_view_GLES20Canvas_drawBitmapMesh(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkBitmap* bitmap, jbyteArray buffer,
        jint meshWidth, jint meshHeight, jfloatArray vertices, jint offset,
        jintArray colors, jint colorOffset, SkPaint* paint) {
    // This object allows the renderer to allocate a global JNI ref to the buffer object.
    JavaHeapBitmapRef bitmapRef(env, bitmap, buffer);

    jfloat* verticesArray = vertices ? env->GetFloatArrayElements(vertices, NULL) + offset : NULL;
    jint* colorsArray = colors ? env->GetIntArrayElements(colors, NULL) + colorOffset : NULL;

    renderer->drawBitmapMesh(bitmap, meshWidth, meshHeight, verticesArray, colorsArray, paint);

    if (vertices) env->ReleaseFloatArrayElements(vertices, verticesArray, 0);
    if (colors) env->ReleaseIntArrayElements(colors, colorsArray, 0);
}

static void android_view_GLES20Canvas_drawPatch(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkBitmap* bitmap, jbyteArray buffer, jbyteArray chunks,
        float left, float top, float right, float bottom, SkPaint* paint) {
    // This object allows the renderer to allocate a global JNI ref to the buffer object.
    JavaHeapBitmapRef bitmapRef(env, bitmap, buffer);

    jbyte* storage = env->GetByteArrayElements(chunks, NULL);
    Res_png_9patch* patch = reinterpret_cast<Res_png_9patch*>(storage);
    Res_png_9patch::deserialize(patch);

    renderer->drawPatch(bitmap, &patch->xDivs[0], &patch->yDivs[0],
            &patch->colors[0], patch->numXDivs, patch->numYDivs, patch->numColors,
            left, top, right, bottom, paint);

    env->ReleaseByteArrayElements(chunks, storage, 0);
}

static void android_view_GLES20Canvas_drawColor(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint color, SkXfermode::Mode mode) {
    renderer->drawColor(color, mode);
}

static void android_view_GLES20Canvas_drawRect(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        SkPaint* paint) {
    renderer->drawRect(left, top, right, bottom, paint);
}

static void android_view_GLES20Canvas_drawRoundRect(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        jfloat rx, jfloat ry, SkPaint* paint) {
    renderer->drawRoundRect(left, top, right, bottom, rx, ry, paint);
}

static void android_view_GLES20Canvas_drawCircle(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat x, jfloat y, jfloat radius, SkPaint* paint) {
    renderer->drawCircle(x, y, radius, paint);
}

static void android_view_GLES20Canvas_drawOval(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        SkPaint* paint) {
    renderer->drawOval(left, top, right, bottom, paint);
}

static void android_view_GLES20Canvas_drawArc(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat left, jfloat top, jfloat right, jfloat bottom,
        jfloat startAngle, jfloat sweepAngle, jboolean useCenter, SkPaint* paint) {
    renderer->drawArc(left, top, right, bottom, startAngle, sweepAngle, useCenter, paint);
}

static void android_view_GLES20Canvas_drawRects(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkRegion* region, SkPaint* paint) {
    SkRegion::Iterator it(*region);
    while (!it.done()) {
        const SkIRect& r = it.rect();
        renderer->drawRect(r.fLeft, r.fTop, r.fRight, r.fBottom, paint);
        it.next();
    }
}

static void android_view_GLES20Canvas_drawPoints(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloatArray points, jint offset, jint count, SkPaint* paint) {
    jfloat* storage = env->GetFloatArrayElements(points, NULL);
    renderer->drawPoints(storage + offset, count, paint);
    env->ReleaseFloatArrayElements(points, storage, 0);
}

static void android_view_GLES20Canvas_drawPath(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkPath* path, SkPaint* paint) {
    renderer->drawPath(path, paint);
}

static void android_view_GLES20Canvas_drawLines(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloatArray points, jint offset, jint count, SkPaint* paint) {
    jfloat* storage = env->GetFloatArrayElements(points, NULL);
    renderer->drawLines(storage + offset, count, paint);
    env->ReleaseFloatArrayElements(points, storage, 0);
}

// ----------------------------------------------------------------------------
// Shaders and color filters
// ----------------------------------------------------------------------------

static void android_view_GLES20Canvas_resetModifiers(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jint modifiers) {
    if (modifiers & MODIFIER_SHADOW) renderer->resetShadow();
    if (modifiers & MODIFIER_SHADER) renderer->resetShader();
    if (modifiers & MODIFIER_COLOR_FILTER) renderer->resetColorFilter();
}

static void android_view_GLES20Canvas_setupShader(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkiaShader* shader) {
    renderer->setupShader(shader);
}

static void android_view_GLES20Canvas_setupColorFilter(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, SkiaColorFilter* filter) {
    renderer->setupColorFilter(filter);
}

static void android_view_GLES20Canvas_setupShadow(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jfloat radius, jfloat dx, jfloat dy, jint color) {
    renderer->setupShadow(radius, dx, dy, color);
}

// ----------------------------------------------------------------------------
// Text
// ----------------------------------------------------------------------------

static void renderText(OpenGLRenderer* renderer, const jchar* text, int count,
        jfloat x, jfloat y, int flags, SkPaint* paint) {
#if RTL_USE_HARFBUZZ
    sp<TextLayoutCacheValue> value = gTextLayoutCache.getValue(
            paint, text, 0, count, count, flags);
    if (value == NULL) {
        LOGE("Cannot get TextLayoutCache value");
        return ;
    }
#if DEBUG_GLYPHS
    logGlyphs(value);
#endif
    const jchar* glyphArray = value->getGlyphs();
    int glyphCount = value->getGlyphsCount();
    int bytesCount = glyphCount * sizeof(jchar);
    renderer->drawText((const char*) glyphArray, bytesCount, glyphCount, x, y, paint);
#else
    const jchar *workText;
    jchar* buffer = NULL;
    int32_t workBytes;
    if (TextLayout::prepareText(paint, text, count, flags, &workText, &workBytes, &buffer)) {
        renderer->drawText((const char*) workText, workBytes, count, x, y, paint);
        free(buffer);
    }
#endif
}

static void renderTextRun(OpenGLRenderer* renderer, const jchar* text,
        jint start, jint count, jint contextCount, jfloat x, jfloat y,
        int flags, SkPaint* paint) {
#if RTL_USE_HARFBUZZ
    sp<TextLayoutCacheValue> value = gTextLayoutCache.getValue(
            paint, text, start, count, contextCount, flags);
    if (value == NULL) {
        LOGE("Cannot get TextLayoutCache value");
        return ;
    }
#if DEBUG_GLYPHS
    logGlyphs(value);
#endif
    const jchar* glyphArray = value->getGlyphs();
    int glyphCount = value->getGlyphsCount();
    int bytesCount = glyphCount * sizeof(jchar);
    renderer->drawText((const char*) glyphArray, bytesCount, glyphCount, x, y, paint);
#else
    uint8_t rtl = flags & 0x1;
    if (rtl) {
        SkAutoSTMalloc<80, jchar> buffer(contextCount);
        jchar* shaped = buffer.get();
        if (TextLayout::prepareRtlTextRun(text, start, count, contextCount, shaped)) {
            renderer->drawText((const char*) shaped, count << 1, count, x, y, paint);
        } else {
            LOGW("drawTextRun error");
        }
    } else {
        renderer->drawText((const char*) (text + start), count << 1, count, x, y, paint);
    }
#endif
}

static void android_view_GLES20Canvas_drawTextArray(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jcharArray text, jint index, jint count,
        jfloat x, jfloat y, jint flags, SkPaint* paint) {
    jchar* textArray = env->GetCharArrayElements(text, NULL);
    renderText(renderer, textArray + index, count, x, y, flags, paint);
    env->ReleaseCharArrayElements(text, textArray, JNI_ABORT);
}

static void android_view_GLES20Canvas_drawText(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jstring text, jint start, jint end,
        jfloat x, jfloat y, jint flags, SkPaint* paint) {
    const jchar* textArray = env->GetStringChars(text, NULL);
    renderText(renderer, textArray + start, end - start, x, y, flags, paint);
    env->ReleaseStringChars(text, textArray);
}

static void android_view_GLES20Canvas_drawTextRunArray(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jcharArray text, jint index, jint count,
        jint contextIndex, jint contextCount, jfloat x, jfloat y, jint dirFlags,
        SkPaint* paint) {
    jchar* textArray = env->GetCharArrayElements(text, NULL);
    renderTextRun(renderer, textArray + contextIndex, index - contextIndex,
            count, contextCount, x, y, dirFlags, paint);
    env->ReleaseCharArrayElements(text, textArray, JNI_ABORT);
 }

static void android_view_GLES20Canvas_drawTextRun(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, jstring text, jint start, jint end,
        jint contextStart, int contextEnd, jfloat x, jfloat y, jint dirFlags,
        SkPaint* paint) {
    const jchar* textArray = env->GetStringChars(text, NULL);
    jint count = end - start;
    jint contextCount = contextEnd - contextStart;
    renderTextRun(renderer, textArray + contextStart, start - contextStart,
            count, contextCount, x, y, dirFlags, paint);
    env->ReleaseStringChars(text, textArray);
}

// ----------------------------------------------------------------------------
// Display lists
// ----------------------------------------------------------------------------

static DisplayList* android_view_GLES20Canvas_getDisplayList(JNIEnv* env,
        jobject clazz, DisplayListRenderer* renderer) {
    return renderer->getDisplayList();
}

static OpenGLRenderer* android_view_GLES20Canvas_getDisplayListRenderer(JNIEnv* env,
        jobject clazz, DisplayListRenderer* renderer) {
    if (renderer == NULL) {
        renderer = new DisplayListRenderer;
    } else {
        renderer->reset();
    }
    return renderer;
}

static void android_view_GLES20Canvas_destroyDisplayList(JNIEnv* env,
        jobject clazz, DisplayList* displayList) {
    delete displayList;
}

static bool android_view_GLES20Canvas_drawDisplayList(JNIEnv* env,
        jobject clazz, OpenGLRenderer* renderer, DisplayList* displayList,
        jint width, jint height, jobject dirty) {
    android::uirenderer::Rect bounds;
    bool redraw = renderer->drawDisplayList(displayList, width, height, bounds);
    if (redraw && dirty != NULL) {
        env->CallVoidMethod(dirty, gRectClassInfo.set,
                int(bounds.left), int(bounds.top), int(bounds.right), int(bounds.bottom));
    }
    return redraw;
}

static void android_view_GLES20Canvas_outputDisplayList(JNIEnv* env,
        jobject clazz, OpenGLRenderer* renderer, DisplayList* displayList) {
    renderer->outputDisplayList(displayList);
}

// ----------------------------------------------------------------------------
// Layers
// ----------------------------------------------------------------------------

static void android_view_GLES20Canvas_interrupt(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    renderer->interrupt();
}

static void android_view_GLES20Canvas_resume(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer) {
    renderer->resume();
}

static OpenGLRenderer* android_view_GLES20Canvas_createLayerRenderer(JNIEnv* env,
        jobject clazz, Layer* layer) {
    if (layer) {
        return new LayerRenderer(layer);
    }
    return NULL;
}

static Layer* android_view_GLES20Canvas_createTextureLayer(JNIEnv* env, jobject clazz,
        jintArray layerInfo) {
    Layer* layer = LayerRenderer::createTextureLayer();

    if (layer) {
        jint* storage = env->GetIntArrayElements(layerInfo, NULL);
        storage[0] = layer->texture;
        env->ReleaseIntArrayElements(layerInfo, storage, 0);
    }

    return layer;
}

static Layer* android_view_GLES20Canvas_createLayer(JNIEnv* env, jobject clazz,
        jint width, jint height, jboolean isOpaque, jintArray layerInfo) {
    Layer* layer = LayerRenderer::createLayer(width, height, isOpaque);

    if (layer) {
        jint* storage = env->GetIntArrayElements(layerInfo, NULL);
        storage[0] = layer->width;
        storage[1] = layer->height;
        env->ReleaseIntArrayElements(layerInfo, storage, 0);
    }

    return layer;
}

static void android_view_GLES20Canvas_resizeLayer(JNIEnv* env, jobject clazz,
        Layer* layer, jint width, jint height, jintArray layerInfo) {
    LayerRenderer::resizeLayer(layer, width, height);

    jint* storage = env->GetIntArrayElements(layerInfo, NULL);
    storage[0] = layer->width;
    storage[1] = layer->height;
    env->ReleaseIntArrayElements(layerInfo, storage, 0);
}

static void android_view_GLES20Canvas_updateTextureLayer(JNIEnv* env, jobject clazz,
        Layer* layer, jint width, jint height, SurfaceTexture* surface) {
    float transform[16];
    surface->updateTexImage();
    surface->getTransformMatrix(transform);
    GLenum renderTarget = surface->getCurrentTextureTarget();

    LayerRenderer::updateTextureLayer(layer, width, height, renderTarget, transform);
}

static void android_view_GLES20Canvas_destroyLayer(JNIEnv* env, jobject clazz, Layer* layer) {
    LayerRenderer::destroyLayer(layer);
}

static void android_view_GLES20Canvas_destroyLayerDeferred(JNIEnv* env,
        jobject clazz, Layer* layer) {
    LayerRenderer::destroyLayerDeferred(layer);
}

static void android_view_GLES20Canvas_drawLayer(JNIEnv* env, jobject clazz,
        OpenGLRenderer* renderer, Layer* layer, jfloat x, jfloat y, SkPaint* paint) {
    renderer->drawLayer(layer, x, y, paint);
}

#endif // USE_OPENGL_RENDERER

// ----------------------------------------------------------------------------
// Common
// ----------------------------------------------------------------------------

static jboolean android_view_GLES20Canvas_isAvailable(JNIEnv* env, jobject clazz) {
#ifdef USE_OPENGL_RENDERER
    return JNI_TRUE;
#else
    return JNI_FALSE;
#endif
}

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

static void
android_app_ActivityThread_dumpGraphics(JNIEnv* env, jobject clazz, jobject javaFileDescriptor)
{
#ifdef USE_OPENGL_RENDERER
    int fd = jniGetFDFromFileDescriptor(env, javaFileDescriptor);
    android::uirenderer::DisplayList::outputLogBuffer(fd);
#endif // USE_OPENGL_RENDERER
}

// ----------------------------------------------------------------------------
// JNI Glue
// ----------------------------------------------------------------------------

const char* const kClassPathName = "android/view/GLES20Canvas";

static JNINativeMethod gMethods[] = {
    { "nIsAvailable",       "()Z",             (void*) android_view_GLES20Canvas_isAvailable },

#ifdef USE_OPENGL_RENDERER
    { "nPreserveBackBuffer", "()Z",            (void*) android_view_GLES20Canvas_preserveBackBuffer },

    { "nCreateRenderer",    "()I",             (void*) android_view_GLES20Canvas_createRenderer },
    { "nDestroyRenderer",   "(I)V",            (void*) android_view_GLES20Canvas_destroyRenderer },
    { "nSetViewport",       "(III)V",          (void*) android_view_GLES20Canvas_setViewport },
    { "nPrepare",           "(IZ)V",           (void*) android_view_GLES20Canvas_prepare },
    { "nPrepareDirty",      "(IIIIIZ)V",       (void*) android_view_GLES20Canvas_prepareDirty },
    { "nFinish",            "(I)V",            (void*) android_view_GLES20Canvas_finish },

    { "nCallDrawGLFunction", "(II)Z",
            (void*) android_view_GLES20Canvas_callDrawGLFunction },

    { "nSave",              "(II)I",           (void*) android_view_GLES20Canvas_save },
    { "nRestore",           "(I)V",            (void*) android_view_GLES20Canvas_restore },
    { "nRestoreToCount",    "(II)V",           (void*) android_view_GLES20Canvas_restoreToCount },
    { "nGetSaveCount",      "(I)I",            (void*) android_view_GLES20Canvas_getSaveCount },

    { "nSaveLayer",         "(IFFFFII)I",      (void*) android_view_GLES20Canvas_saveLayer },
    { "nSaveLayerAlpha",    "(IFFFFII)I",      (void*) android_view_GLES20Canvas_saveLayerAlpha },

    { "nQuickReject",       "(IFFFFI)Z",       (void*) android_view_GLES20Canvas_quickReject },
    { "nClipRect",          "(IFFFFI)Z",       (void*) android_view_GLES20Canvas_clipRectF },
    { "nClipRect",          "(IIIIII)Z",       (void*) android_view_GLES20Canvas_clipRect },

    { "nTranslate",         "(IFF)V",          (void*) android_view_GLES20Canvas_translate },
    { "nRotate",            "(IF)V",           (void*) android_view_GLES20Canvas_rotate },
    { "nScale",             "(IFF)V",          (void*) android_view_GLES20Canvas_scale },
    { "nSkew",              "(IFF)V",          (void*) android_view_GLES20Canvas_skew },

    { "nSetMatrix",         "(II)V",           (void*) android_view_GLES20Canvas_setMatrix },
    { "nGetMatrix",         "(I)I",            (void*) android_view_GLES20Canvas_getNativeMatrix },
    { "nGetMatrix",         "(II)V",           (void*) android_view_GLES20Canvas_getMatrix },
    { "nConcatMatrix",      "(II)V",           (void*) android_view_GLES20Canvas_concatMatrix },

    { "nDrawBitmap",        "(II[BFFI)V",      (void*) android_view_GLES20Canvas_drawBitmap },
    { "nDrawBitmap",        "(II[BFFFFFFFFI)V",(void*) android_view_GLES20Canvas_drawBitmapRect },
    { "nDrawBitmap",        "(II[BII)V",       (void*) android_view_GLES20Canvas_drawBitmapMatrix },

    { "nDrawBitmapMesh",    "(II[BII[FI[III)V",(void*) android_view_GLES20Canvas_drawBitmapMesh },

    { "nDrawPatch",         "(II[B[BFFFFI)V",  (void*) android_view_GLES20Canvas_drawPatch },

    { "nDrawColor",         "(III)V",          (void*) android_view_GLES20Canvas_drawColor },
    { "nDrawRect",          "(IFFFFI)V",       (void*) android_view_GLES20Canvas_drawRect },
    { "nDrawRects",         "(III)V",          (void*) android_view_GLES20Canvas_drawRects },
    { "nDrawRoundRect",     "(IFFFFFFI)V",     (void*) android_view_GLES20Canvas_drawRoundRect },
    { "nDrawCircle",        "(IFFFI)V",        (void*) android_view_GLES20Canvas_drawCircle },
    { "nDrawOval",          "(IFFFFI)V",       (void*) android_view_GLES20Canvas_drawOval },
    { "nDrawArc",           "(IFFFFFFZI)V",    (void*) android_view_GLES20Canvas_drawArc },
    { "nDrawPoints",        "(I[FIII)V",       (void*) android_view_GLES20Canvas_drawPoints },

    { "nDrawPath",          "(III)V",          (void*) android_view_GLES20Canvas_drawPath },
    { "nDrawLines",         "(I[FIII)V",       (void*) android_view_GLES20Canvas_drawLines },

    { "nResetModifiers",    "(II)V",           (void*) android_view_GLES20Canvas_resetModifiers },
    { "nSetupShader",       "(II)V",           (void*) android_view_GLES20Canvas_setupShader },
    { "nSetupColorFilter",  "(II)V",           (void*) android_view_GLES20Canvas_setupColorFilter },
    { "nSetupShadow",       "(IFFFI)V",        (void*) android_view_GLES20Canvas_setupShadow },

    { "nDrawText",          "(I[CIIFFII)V",    (void*) android_view_GLES20Canvas_drawTextArray },
    { "nDrawText",          "(ILjava/lang/String;IIFFII)V",
            (void*) android_view_GLES20Canvas_drawText },

    { "nDrawTextRun",       "(I[CIIIIFFII)V",  (void*) android_view_GLES20Canvas_drawTextRunArray },
    { "nDrawTextRun",       "(ILjava/lang/String;IIIIFFII)V",
            (void*) android_view_GLES20Canvas_drawTextRun },

    { "nGetClipBounds",     "(ILandroid/graphics/Rect;)Z",
            (void*) android_view_GLES20Canvas_getClipBounds },

    { "nGetDisplayList",         "(I)I",       (void*) android_view_GLES20Canvas_getDisplayList },
    { "nDestroyDisplayList",     "(I)V",       (void*) android_view_GLES20Canvas_destroyDisplayList },
    { "nGetDisplayListRenderer", "(I)I",       (void*) android_view_GLES20Canvas_getDisplayListRenderer },
    { "nDrawDisplayList",        "(IIIILandroid/graphics/Rect;)Z",
                                               (void*) android_view_GLES20Canvas_drawDisplayList },
    { "nOutputDisplayList",      "(II)V",      (void*) android_view_GLES20Canvas_outputDisplayList },
    { "nInterrupt",              "(I)V",       (void*) android_view_GLES20Canvas_interrupt },
    { "nResume",                 "(I)V",       (void*) android_view_GLES20Canvas_resume },

    { "nCreateLayerRenderer",    "(I)I",       (void*) android_view_GLES20Canvas_createLayerRenderer },
    { "nCreateLayer",            "(IIZ[I)I",   (void*) android_view_GLES20Canvas_createLayer },
    { "nResizeLayer",            "(III[I)V" ,  (void*) android_view_GLES20Canvas_resizeLayer },
    { "nCreateTextureLayer",     "([I)I",      (void*) android_view_GLES20Canvas_createTextureLayer },
    { "nUpdateTextureLayer",     "(IIII)V",    (void*) android_view_GLES20Canvas_updateTextureLayer },
    { "nDestroyLayer",           "(I)V",       (void*) android_view_GLES20Canvas_destroyLayer },
    { "nDestroyLayerDeferred",   "(I)V",       (void*) android_view_GLES20Canvas_destroyLayerDeferred },
    { "nDrawLayer",              "(IIFFI)V",   (void*) android_view_GLES20Canvas_drawLayer },

#endif
};

static JNINativeMethod gActivityThreadMethods[] = {
    { "dumpGraphicsInfo",        "(Ljava/io/FileDescriptor;)V",
                                               (void*) android_app_ActivityThread_dumpGraphics }
};


#ifdef USE_OPENGL_RENDERER
    #define FIND_CLASS(var, className) \
            var = env->FindClass(className); \
            LOG_FATAL_IF(! var, "Unable to find class " className);

    #define GET_METHOD_ID(var, clazz, methodName, methodDescriptor) \
            var = env->GetMethodID(clazz, methodName, methodDescriptor); \
            LOG_FATAL_IF(! var, "Unable to find method " methodName);
#else
    #define FIND_CLASS(var, className)
    #define GET_METHOD_ID(var, clazz, methodName, methodDescriptor)
#endif

int register_android_view_GLES20Canvas(JNIEnv* env) {
    jclass clazz;
    FIND_CLASS(clazz, "android/graphics/Rect");
    GET_METHOD_ID(gRectClassInfo.set, clazz, "set", "(IIII)V");

    return AndroidRuntime::registerNativeMethods(env, kClassPathName, gMethods, NELEM(gMethods));
}

const char* const kActivityThreadPathName = "android/app/ActivityThread";

int register_android_app_ActivityThread(JNIEnv* env)
{
    return AndroidRuntime::registerNativeMethods(
            env, kActivityThreadPathName,
            gActivityThreadMethods, NELEM(gActivityThreadMethods));
}

};