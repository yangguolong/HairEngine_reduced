LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ASM_static_prebuilt
LOCAL_SRC_FILES := $(LOCAL_PATH)/stasm/libASM.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/stasm
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := AMD_static_prebuilt
LOCAL_SRC_FILES := $(LOCAL_PATH)/TexGenerator/umfpacklib/merge/libamd.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := UMFPACK_static_prebuilt
LOCAL_SRC_FILES := $(LOCAL_PATH)/TexGenerator/umfpacklib/merge/libumfpack.a
include $(PREBUILT_STATIC_LIBRARY)

My_GLM_INCLUDE_DIR := $(LOCAL_PATH)/glm $(LOCAL_PATH)/glm/core $(LOCAL_PATH)/glm/gtc $(LOCAL_PATH)/glm/gtx $(LOCAL_PATH)/glm/virtrev
My_UtilitySRC := $(wildcard Utility/ShaderUtility/*.cpp) $(wildcard Utility/ImageUtility/*.cpp) 
My_ASMLibSRC := $(wildcard stasm/*.cpp) $(wildcard stasm/MOD_1/*.cpp)
My_HalfFloatSRC := $(wildcard HalfFloat/*.cpp)
My_FaceRegressorSRC := $(wildcard FaceRegressor/*.cpp) $(wildcard FaceRegressor/MyCommon/*.cpp) $(wildcard FaceRegressor/MyCommon/Math/*.cpp) $(wildcard FaceRegressor/MyCommon/Time/*.cpp) $(wildcard FaceRegressor/Holder/*.cpp)
My_ModelLoaderSRC := $(wildcard ModelLoader/*.cpp)	
My_PAINTSELECTIONSRC := $(wildcard PaintSelectionAlgorithm/*.cpp) $(wildcard PaintSelectionAlgorithm/fgmm/*.cpp)
My_HeadDetectorSRC := $(wildcard HeadDetector/*.cpp)
My_HeadGeneratorSRC := $(wildcard HeadGenerator/*.cpp)
My_TexGeneratorSRC = $(wildcard TexGenerator/*.cpp)
My_RendererSRC := $(wildcard Renderer/*.cpp)
My_HairEngineSRC := $(wildcard HairEngine/*.cpp)
My_JniSRC := $(wildcard JniPart/*.cpp)

include $(CLEAR_VARS)
OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on
include $(OPENCV_ANDROID_SDK_HOME)/sdk/native/jni/OpenCV.mk

LOCAL_MODULE := HairEngineJni
LOCAL_SRC_FILES += $(My_UtilitySRC) \
				  $(My_ASMLibSRC) \
				  $(My_HalfFloatSRC) \
				  $(My_FaceRegressorSRC) \
				  $(My_ModelLoaderSRC) \
				  $(My_PAINTSELECTIONSRC) \
				  $(My_HeadDetectorSRC) \
				  $(My_HeadGeneratorSRC) \
				  $(My_TexGeneratorSRC) \
				  $(My_RendererSRC) \
				  $(My_HairEngineSRC) \
				  $(My_JniSRC) 
LOCAL_C_INCLUDES += $(My_GLM_INCLUDE_DIR) \
					$(LOCAL_PATH)/Utility/ImageUtility \
					$(LOCAL_PATH)/Utility/ShaderUtility \
					$(LOCAL_PATH)/stasm \
					$(LOCAL_PATH)/stasm/MOD_1 \
					$(LOCAL_PATH)/HalfFloat \
					$(LOCAL_PATH)/FaceRegressor/ \
					$(LOCAL_PATH)/FaceRegressor/MyCommon \
					$(LOCAL_PATH)/FaceRegressor/MyCommon/Math \
					$(LOCAL_PATH)/FaceRegressor/MyCommon/Time \
					$(LOCAL_PATH)/FaceRegressor/Holder \
					$(LOCAL_PATH)/TexGenerator/umfpacklib/include \
					$(LOCAL_PATH)/ModelLoader \
					$(LOCAL_PATH)/PaintSelectionAlgorithm \
					$(LOCAL_PATH)/PaintSelectionAlgorithm/fgmm \
					$(LOCAL_PATH)/HeadDetector \
					$(LOCAL_PATH)/HeadGenerator \
					$(LOCAL_PATH)/TexGenerator \
					$(LOCAL_PATH)/TexGenerator/umfpacklib/include \
					$(LOCAL_PATH)/Renderer \
					$(LOCAL_PATH)/HairEngine
					
LOCAL_STATIC_LIBRARIES += ASM_static_prebuilt UMFPACK_static_prebuilt AMD_static_prebuilt
LOCAL_LDLIBS +=  -llog -lGLESv1_CM -lGLESv2 -lEGL -ljnigraphics
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES=1
#LOCAL_CFLAGS += -Wall
include $(BUILD_SHARED_LIBRARY)
