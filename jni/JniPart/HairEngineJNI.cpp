#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "HairEngine.h"

#define LOG_TAG "HairEngineJni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} argb;


#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_initEngine
 * Signature: (Ljava/lang/String;)I
 */

// 获取包签名
int getSignHashCode(JNIEnv* pEnv, jobject context)
{
	//Context的类
	jclass context_clazz = pEnv->GetObjectClass(context);
	// 得到 getPackageManager 方法的 ID
	jmethodID methodID_getPackageManager = pEnv->GetMethodID(context_clazz,
			"getPackageManager", "()Landroid/content/pm/PackageManager;");

	assert(methodID_getPackageManager!=NULL);

	// 获得PackageManager对象
	jobject packageManager = pEnv->CallObjectMethod(context,
			methodID_getPackageManager);

//	// 获得 PackageManager 类
	jclass pm_clazz = pEnv->GetObjectClass(packageManager);

	// 得到 getPackageInfo 方法的 ID
	jmethodID methodID_pm = pEnv->GetMethodID(pm_clazz, "getPackageInfo",
			"(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

//
//	// 得到 getPackageName 方法的 ID
	jmethodID methodID_pack = pEnv->GetMethodID(context_clazz,
			"getPackageName", "()Ljava/lang/String;");

	// 获得当前应用的包名
	jstring application_package = (jstring)pEnv->CallObjectMethod(context,
			methodID_pack);
	const char *str = pEnv->GetStringUTFChars(application_package, 0);
	LOGI("getSignHashCode: packageName: %s\n", str);

	// 获得PackageInfo
	jobject packageInfo = pEnv->CallObjectMethod(packageManager,
			methodID_pm, application_package, 64);

	jclass packageinfo_clazz = pEnv->GetObjectClass(packageInfo);
	jfieldID fieldID_signatures = pEnv->GetFieldID(packageinfo_clazz,
			"signatures", "[Landroid/content/pm/Signature;");
	jobjectArray signature_arr = (jobjectArray)pEnv->GetObjectField(
			packageInfo, fieldID_signatures);
	//Signature数组中取出第一个元素
	jobject signature = pEnv->GetObjectArrayElement(signature_arr, 0);

	//读signature的hashcode
	jclass signature_clazz = pEnv->GetObjectClass(signature);
	jmethodID methodID_hashcode = pEnv->GetMethodID(signature_clazz,
			"hashCode", "()I");
	jint hashCode = pEnv->CallIntMethod(signature, methodID_hashcode);

	return hashCode;
}

JNIEXPORT jint JNICALL Java_com_gaps_hairengine_HairEngine_initEngineHE
(JNIEnv * pEnv, jobject pObject, jstring pDataDir, jboolean useRegressor)
{
	//校验签名
	jclass class_HairEngine = pEnv->GetObjectClass(pObject);
	jfieldID field_context = pEnv->GetFieldID(class_HairEngine, "m_context", "Landroid/content/Context;");
	//= jfieldID field_context = pEnv->GetFieldID(class_HairEngine, "m_context", "android/content/Context");
	jobject object_context = pEnv->GetObjectField(pObject, field_context);

	int sigCode = getSignHashCode(pEnv, object_context);
	LOGI("HE_initEngine: sigCode: %d\n", sigCode);
	if(sigCode != 1764256230
	&& sigCode != 1210824553
	&& sigCode != -95090613
	&& sigCode != -1275985344
	&& sigCode != 1175902095
	&& sigCode != 274733131
	&& sigCode != 2042837435
	&& sigCode != 1713000916
	&& sigCode != 20333306
	&& sigCode != 199565991)
	{
		LOGE("HE_initEngine： signature error.\n");
		return 0;
	}

    const char* dataDir = pEnv->GetStringUTFChars(pDataDir, NULL);
    
    HairEngine * hairEngine = new HairEngine();
    if(!hairEngine)
    {
        LOGE("HE_initEngine: Failed to create HairEngine instance.\n");
        return 0;
    }
    
    bool isSuccess = hairEngine->initEngine(dataDir, useRegressor);
    
    pEnv->ReleaseStringUTFChars(pDataDir, dataDir);
    
    if(!isSuccess)
    {
        delete hairEngine;
        LOGE("HE_initEngine: failed to init HairEngine.\n");
        return 0;
    }
    else
    {
        LOGI("HE_initEngine: Successful to create HairEngine instance.\n");
        return (int)hairEngine;
    }
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_setImage
 * Signature: (ILandroid/graphics/Bitmap;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_setImageHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jobject pBitmap, jstring pWorkDir)
{
    if(pHandle == 0)
    {
        LOGE("HE_clearStroke: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    AndroidBitmapInfo infocolor;
    void *  pixelscolor;
    int ret;
    
    if ((ret = AndroidBitmap_getInfo(pEnv, pBitmap, &infocolor)) < 0)
    {
        LOGE("HE_setImage: AndroidBitmap_getInfo() failed ! error=%d", ret);
        return false;
    }
    
    if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888 )
    {
        char errorInfo[500];
        switch (infocolor.format) {
            case ANDROID_BITMAP_FORMAT_NONE:
                sprintf(errorInfo, "ANDROID_BITMAP_FORMAT_NONE");
                break;
            case ANDROID_BITMAP_FORMAT_RGBA_8888:
                sprintf(errorInfo, "ANDROID_BITMAP_FORMAT_RGBA_8888");
                break;
            case ANDROID_BITMAP_FORMAT_RGB_565:
                sprintf(errorInfo, "ANDROID_BITMAP_FORMAT_RGB_565");
                break;
            case ANDROID_BITMAP_FORMAT_RGBA_4444:
                sprintf(errorInfo, "ANDROID_BITMAP_FORMAT_RGBA_4444");
                break;
            case ANDROID_BITMAP_FORMAT_A_8:
                sprintf(errorInfo, "ANDROID_BITMAP_FORMAT_A_8");
                break;
                
            default:
                sprintf(errorInfo, "Unknow format");
                break;
        }
        LOGE("Bitmap format is not RGBA_8888! (it is %s)", errorInfo);
        return false;
    }
    
    if ((ret = AndroidBitmap_lockPixels(pEnv, pBitmap, &pixelscolor)) < 0)
    {
        LOGE("HE_setImage: AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return false;
    }
    
    unsigned char * pixelBuffer = (unsigned char *)malloc(infocolor.width * infocolor.height * 4 * sizeof(unsigned char));
    
    int arrayIndex = 0;
    for (unsigned int y=0; y<infocolor.height; y++)
    {
        argb * line = (argb *)pixelscolor;
        for (unsigned int x=0; x<infocolor.width; x++)
        {
            pixelBuffer[arrayIndex++] = line[x].red;
            pixelBuffer[arrayIndex++] = line[x].green;
            pixelBuffer[arrayIndex++] = line[x].blue;
            pixelBuffer[arrayIndex++] = line[x].alpha;
        }
        
        pixelscolor = (unsigned char *)pixelscolor + infocolor.stride;
    }
    
    AndroidBitmap_unlockPixels(pEnv, pBitmap);
    
    const char* workDir = pEnv->GetStringUTFChars(pWorkDir, NULL);
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    bool isSuccess = pEngine->setImage(pixelBuffer, infocolor.width, infocolor.height, workDir);
    
    LOGI("HE_setImage: %s (w, h) = (%d %d), str(%s)\n", isSuccess?"success":"error", infocolor.width, infocolor.height, workDir);
    
    pEnv->ReleaseStringUTFChars(pWorkDir, workDir);
    
    free(pixelBuffer);
    
    return isSuccess;
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_addStroke
 * Signature: (III[F)[F
 */
JNIEXPORT jfloatArray JNICALL Java_com_gaps_hairengine_HairEngine_addStrokeHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint pType, jint pSize, jfloatArray pPoints)
{
    if(pHandle == 0)
    {
        LOGE("HE_clearStroke: invalid pHandle %d\n", pHandle);
        return NULL;
    }
    
    jsize lLength = pEnv->GetArrayLength(pPoints);
    float * lArray = (float *)malloc(lLength * sizeof(float));
    pEnv->GetFloatArrayRegion(pPoints, 0, lLength, lArray);
    if (pEnv->ExceptionCheck())
    {
        free(lArray);
        return NULL;
    }
    
    //translate float array to int array
    int * intArray = (int *)malloc(lLength * sizeof(int));
    for(int i=0; i<lLength; i++)
        intArray[i] = (int)lArray[i];
    free(lArray);
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    HEContour * pContour = pEngine->addStroke(pType, pSize, intArray);
    free(intArray);
    
    // warp result in one float array
    int contourNum = pContour->contourNum;
    if(contourNum <= 0)
        return NULL;
    
    int totalPointsNum = pContour->totalPointsNum;
    int arrayLength = contourNum + 1 + totalPointsNum * 2;
    float * rstArray = (float *)malloc(arrayLength * sizeof(float));
    
    int arrayIndex = 0;
    rstArray[arrayIndex++] = (float)contourNum;
    for(int i=0; i<contourNum; i++)
        rstArray[arrayIndex++] = (float)(pContour->contourPointNumArray[i]);
    for(int i=0; i<totalPointsNum; i++)
    {
        rstArray[arrayIndex++] = pContour->contourPoints[i].x;
        rstArray[arrayIndex++] = pContour->contourPoints[i].y;
    }
    
    jfloatArray lJavaArray = pEnv->NewFloatArray(arrayLength);
    if (lJavaArray == NULL)
    {
        free(rstArray);
        return NULL;
    }
    pEnv->SetFloatArrayRegion(lJavaArray, 0, arrayLength, rstArray);

    free(rstArray);

    return lJavaArray;
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_clearStroke
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_clearStrokeHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_clearStroke: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->clearStroke();
}
    
void printEGLConfigAttribs(EGLDisplay display, EGLConfig config)
{
    LOGI("******** config info********\n");
    int value;
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &value);
    LOGI("EGL_SURFACE_TYPE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &value);
    LOGI("EGL_SURFACE_TYPE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE, &value);
    LOGI("EGL_COLOR_BUFFER_TYPE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_BUFFER_SIZE, &value);
    LOGI("EGL_BUFFER_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &value);
    LOGI("EGL_RED_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &value);
    LOGI("EGL_GREEN_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &value);
    LOGI("EGL_BLUE_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &value);
    LOGI("EGL_ALPHA_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &value);
    LOGI("EGL_DEPTH_SIZE: %d\n", value);
    
    eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA, &value);
    LOGI("EGL_BIND_TO_TEXTURE_RGBA: %d\n", value);
    
    LOGI("**************************\n\n");
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_detectFace
 * Signature: (I)Z
 */

JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_detectFaceHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
	HairEngine * pEngine = (HairEngine *)pHandle;
	return pEngine->detectFace();
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_autoGenerateHead
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_autoGenerateHeadHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
	//LOGI("Flag 1.\n");

	    if(pHandle == 0)
	    {
	        LOGE("HE_finishStroke: invalid pHandle %d\n", pHandle);
	        return false;
	    }

	    const EGLint attribs[] = {
	        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
	        EGL_BUFFER_SIZE, 32,
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_ALPHA_SIZE, 8,
	        EGL_DEPTH_SIZE, 16,
	        //EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
	        //EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
	        EGL_NONE
	    };

	    const EGLint attribListPbuffer[] = {
	        EGL_WIDTH, 1,
	        EGL_HEIGHT, 1,
	        EGL_NONE
	    };

	    const EGLint attribListContext[] = {
	        EGL_CONTEXT_CLIENT_VERSION, 2,
	        EGL_NONE
	    };

	    EGLint format;
	    EGLint numConfigs;
	    EGLConfig config;
	    EGLSurface surface; // pixel buffer surface
	    EGLContext context;
	    EGLDisplay display;

	    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	    if(display == EGL_NO_DISPLAY)
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to create GL display (error: %d)\n", ec);
	        return false;
	    }

	    if(!eglInitialize(display, NULL, NULL))
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to init GL display (error: %d)\n", ec);

	        eglTerminate(display);
	        return false;
	    }

	    LOGI("EGL_CLIENT_APIs:\n%s\n", eglQueryString(display, EGL_CLIENT_APIS));
	    LOGI("EGL_EXTENSIONs:\n%s\n", eglQueryString(display, EGL_EXTENSIONS));
	    LOGI("EGL_VENDOR:\n%s\n", eglQueryString(display, EGL_VENDOR));
	    LOGI("EGL_VERSION:\n%s\n", eglQueryString(display, EGL_VERSION));
	    LOGI("Test 4");

	    EGLConfig tmpConfigs[30];
	    int tmpConfigNum;
	    if(!eglGetConfigs(display, tmpConfigs, 30, &tmpConfigNum))
	    {
	        LOGE("Failed to get all configs.\n");
	    }
	    else
	    {
	        for(int i=0; i<tmpConfigNum; i++)
	        {
	            LOGI("for config %d\n", i);
	            printEGLConfigAttribs(display, tmpConfigs[i]);
	        }
	        LOGI("total config num: %d\n", tmpConfigNum);
	    }

	    //if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs))
	    if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || numConfigs <= 0)
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to chooseConfig (error: %d numConfigs:%d)\n", ec, numConfigs);

	        eglTerminate(display);
	        return false;
	    }
	    //EGL_NATIVE_VISUAL_ID

	    if(!eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &format))
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to getConfigAttrib (error: %d)\n", ec);

	        eglTerminate(display);
	        return false;
	    }
	    LOGI("format = %d", format);

	    surface = eglCreatePbufferSurface(display, config, attribListPbuffer);
	    if(surface == EGL_NO_SURFACE)
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to create GL surface (error: %d)\n", ec);

	        eglTerminate(display);
	        return false;
	    }

	    LOGI("watch 1");

	    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribListContext);
	    if(context == EGL_NO_CONTEXT)
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to create GL context (error: %d)\n", ec);

	        eglDestroySurface(display, surface);
	        eglTerminate(display);
	        return false;
	    }

	    LOGI("watch 2");

	    if(!eglMakeCurrent(display, surface, surface, context))
	    {
	        EGLint ec = eglGetError();
	        LOGE("HE_finishStroke: failed to make current (error: %d)\n", ec);

	        eglDestroyContext(display, context);
	        eglDestroySurface(display, surface);
	        eglTerminate(display);
	        return false;
	    }

	    LOGI("watch 3");

	    //LOGI("Flag 2.\n");

	    HairEngine * pEngine = (HairEngine *)pHandle;

	    LOGI("watch 4");

	    bool isSuccess = pEngine->autoGenerateHead(500.0f);

	    LOGI("watch 5: %s", isSuccess?"true":"false");

	    //LOGI("Flag 3.\n");

	    eglDestroyContext(display, context);
	    eglDestroySurface(display, surface);
	    eglTerminate(display);

	    //LOGI("Flag 4.\n");
	    LOGI("watch 6");

	    return isSuccess;
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_finishStroke
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_finishStrokeHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    //LOGI("Flag 1.\n");
    
    if(pHandle == 0)
    {
        LOGE("HE_finishStroke: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_BUFFER_SIZE, 32,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        //EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
        //EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
        EGL_NONE
    };
    
    const EGLint attribListPbuffer[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    
    const EGLint attribListContext[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface; // pixel buffer surface
    EGLContext context;
    EGLDisplay display;
    
    /*
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribListContext);
    
    if(context == EGL_NO_CONTEXT)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL context (error: %d)\n", ec);

        eglTerminate(display);
        return false;
    }
    surface = eglCreatePbufferSurface(display, config, attribListPbuffer);
    if(surface == EGL_NO_SURFACE)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL surface (error: %d)\n", ec);
        
        eglTerminate(display);
        eglDestroyContext(display, context);
        return false;
    }
    if(!eglMakeCurrent(display, surface, surface, context))
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to make current (error: %d)\n", ec);
        
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return false;
     }
    */
    
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL display (error: %d)\n", ec);
        return false;
    }
    
    if(!eglInitialize(display, NULL, NULL))
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to init GL display (error: %d)\n", ec);
        
        eglTerminate(display);
        return false;
    }
    
    LOGI("EGL_CLIENT_APIs:\n%s\n", eglQueryString(display, EGL_CLIENT_APIS));
    LOGI("EGL_EXTENSIONs:\n%s\n", eglQueryString(display, EGL_EXTENSIONS));
    LOGI("EGL_VENDOR:\n%s\n", eglQueryString(display, EGL_VENDOR));
    LOGI("EGL_VERSION:\n%s\n", eglQueryString(display, EGL_VERSION));
    LOGI("Test 4");
    
    EGLConfig tmpConfigs[30];
    int tmpConfigNum;
    if(!eglGetConfigs(display, tmpConfigs, 30, &tmpConfigNum))
    {
        LOGE("Failed to get all configs.\n");
    }
    else
    {
        for(int i=0; i<tmpConfigNum; i++)
        {
            LOGI("for config %d\n", i);
            printEGLConfigAttribs(display, tmpConfigs[i]);
        }
        LOGI("total config num: %d\n", tmpConfigNum);
    }
    
    //if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs))
    if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || numConfigs <= 0)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to chooseConfig (error: %d numConfigs:%d)\n", ec, numConfigs);
        
        eglTerminate(display);
        return false;
    }
    //EGL_NATIVE_VISUAL_ID
    
    if(!eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &format))
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to getConfigAttrib (error: %d)\n", ec);
        
        eglTerminate(display);
        return false;
    }
    LOGI("format = %d", format);
    
    surface = eglCreatePbufferSurface(display, config, attribListPbuffer);
    if(surface == EGL_NO_SURFACE)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL surface (error: %d)\n", ec);
        
        eglTerminate(display);
        return false;
    }
    
    LOGI("watch 1");
    
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribListContext);
    if(context == EGL_NO_CONTEXT)
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL context (error: %d)\n", ec);
        
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return false;
    }
    
    LOGI("watch 2");
    
    if(!eglMakeCurrent(display, surface, surface, context))
    {
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to make current (error: %d)\n", ec);
        
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return false;
    }
    
    LOGI("watch 3");
    
    //LOGI("Flag 2.\n");
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    
    LOGI("watch 4");
    
    bool isSuccess = pEngine->finishStroke(500.0f);
    
    LOGI("watch 5: %s", isSuccess?"true":"false");
    
    //LOGI("Flag 3.\n");
    
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    
    //LOGI("Flag 4.\n");
    LOGI("watch 6");
    
    return isSuccess;
}
    
/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    finishStrokeStep1HE
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_finishStrokeStep1HE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_finishStrokeStep1: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->finishStrokeStep1();
}


/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    finishStrokeStep2HE
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_finishStrokeStep2HE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_finishStroke: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    const EGLint attribListPbuffer[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    
    const EGLint attribListContext[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface; // pixel buffer surface
    EGLContext context;
    EGLDisplay display;
    
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribListContext);
    
    if(context == EGL_NO_CONTEXT)
    {
        eglTerminate(display);
        EGLint ec = eglGetError();
        
        LOGE("HE_finishStroke: failed to create GL context (error: %d)\n", ec);
        return false;
    }
    surface = eglCreatePbufferSurface(display, config, attribListPbuffer);
    if(surface == EGL_NO_SURFACE)
    {
        eglTerminate(display);
        eglDestroyContext(display, context);
        
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to create GL surface (error: %d)\n", ec);
        return false;
    }
    if(!eglMakeCurrent(display, surface, surface, context))
    {
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        
        EGLint ec = eglGetError();
        LOGE("HE_finishStroke: failed to make current (error: %d)\n", ec);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    bool isSuccess = pEngine->finishStrokeStep2(500.0f);
    
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    
    return isSuccess;
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_initViewer
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_initViewerHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint pWidth, jint pHeight)
{
    if(pHandle == 0)
    {
        LOGE("HE_initViewer: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->initViewer(pWidth, pHeight);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_closeViewer
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_closeViewerHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_closeViewer: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->closeViewer();
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_resizeViewer
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_resizeViewerHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint pWidth, jint pHeight)
{
    if(pHandle == 0)
    {
        LOGE("HE_resizeViewer: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->resizeViewer(pWidth, pHeight);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_setHairDir
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_setHairDirHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jstring pHairDir)
{
    const char* hairDir = pEnv->GetStringUTFChars(pHairDir, NULL);
    
    if(pHandle == 0)
    {
        LOGE("HE_setHairDir: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    bool isSuccess = pEngine->setHairDir(hairDir);
    pEnv->ReleaseStringUTFChars(pHairDir, hairDir);

    return isSuccess;
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_setTransform
 * Signature: (IFFF)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_setTransformHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jfloat pXAngle, jfloat pYAngle, jfloat pScale)
{
    if(pHandle == 0)
    {
        LOGE("HE_setTransform: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->setTransform(pXAngle, pYAngle, pScale);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_resetTransform
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_resetTransformHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_resetTransform: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->resetTransform();
}
    
/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    adjustHairPosition
 * Signature: (IFFF)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_adjustHairPositionHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jfloat disX, jfloat disY, jfloat disZ)
{
    if(pHandle == 0)
    {
        LOGE("HE_adjustHairPosition: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->adjustHairPosition(disX, disY, disZ);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    adjustHairScale
 * Signature: (IFFF)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_adjustHairScaleHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jfloat scale)
{
    if(pHandle == 0)
    {
        LOGE("HE_adjustHairScale: invalid pHandle %d\n", pHandle);
        return false;
    }

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->adjustHairScale(scale);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    resetHairPosition
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_resetHairPositionHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_resetHairPosition: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->resetHairPosition();
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_enableShadow
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_enableShadowHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jboolean pEnable)
{
    if(pHandle == 0)
    {
        LOGE("HE_enableShadow: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->enableShadow(pEnable);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_render
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_renderHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("HE_render: invalid pHandle %d\n", pHandle);
        return false;
    }
    
    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->render();
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_setDermabrasionDegree
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_setDermabrasionDegreeHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint degree)
{
    if(pHandle == 0)
    {
        LOGE("HE_setDermabrasionDegree: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_setDermabrasionDegree: set dermabrasion degree:%d", degree);

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->setDermabrasionDegree(degree);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_enableExpression
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_enableExpressionHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jboolean pEnable)
{
    if(pHandle == 0)
    {
        LOGE("HE_enableExpression: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_enableExpression: enable expression:%d", pEnable);

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->enableExpression(pEnable);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_changeExpression
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_changeExpressionHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint expressionID)
{
    if(pHandle == 0)
    {
        LOGE("HE_changeExpression: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_changeExpression: set expression id:%d", expressionID);

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->changeExpression(expressionID);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    HE_changeModifier
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_changeModifierHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jint modifierID)
{
    if(pHandle == 0)
    {
        LOGE("HE_changeModifier: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_changeModifier: set modifier id:%d", modifierID);

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->changeModifier(modifierID);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    changeColorHE
 * Signature: (IFFFF)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_changeHairColorHE
(JNIEnv * pEnv, jobject pObject, jint pHandle, jfloat r, jfloat g, jfloat b, jfloat a)
{
    if(pHandle == 0)
    {
        LOGE("changeColor: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_changeColor: set color :%f, %f, %f, %f\n", r, g, b, a);

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->changeHairColor(r, g, b, a);
}

/*
 * Class:     com_gaps_hairengine_HairEngine
 * Method:    resetExpressionHE
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_gaps_hairengine_HairEngine_resetExpressionHE
(JNIEnv * pEnv, jobject pObject, jint pHandle)
{
    if(pHandle == 0)
    {
        LOGE("changeColor: invalid pHandle %d\n", pHandle);
        return false;
    }

    LOGI("HE_resetExpression\n");

    HairEngine * pEngine = (HairEngine *)pHandle;
    return pEngine->resetExpression();
}

#ifdef __cplusplus
}
#endif
