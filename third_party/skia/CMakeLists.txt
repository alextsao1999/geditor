# Created By Alex Tsao 2019/9/21
cmake_minimum_required(VERSION 3.10.2)
set(BUILD_SHARED OFF)
if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "-fstack-protector -fno-strict-aliasing -fno-rtti -fPIC -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "-fstack-protector -fno-strict-aliasing -fPIC -fvisibility=hidden ${CMAKE_C_FLAGS}")
endif ()
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-D_LARGEFILE64_SOURCE)
add_definitions("-DSKIA_IMPLEMENTATION=1")
add_definitions("-DGR_IMPLEMENTATION=1")
#add_definitions("-DGR_QNX_BUILD=1")
#add_definitions("-DUSE_GANESH=1")
add_definitions("-D_CRT_SECURE_NO_WARNINGS")
add_definitions("-DSK_IGNORE_ETC1_SUPPORT")
#add_definitions("-DUSE_MINGW")
file(GLOB_RECURSE skia_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)
set(inc_dirs
		animator;config;core;device/xps;effects;gpu;gpu/gl;images;doc;pathops;pdf;pipe;ports;
		record;svg;utils;utils/win;utils/ios;utils/mac;views;views/android;views/animated;views/unix;xml)
foreach(inc_dir ${inc_dirs})
	list(APPEND skia_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/include/${inc_dir}")
endforeach()
set(inc_dirs animator;core;effects;effects/gradients;fonts;gpu;gpu/effects/;gpu/gl/;gpu/gl/builder;gpu/gl/debug;
		image;images;lazy;opts;pathops;pipe;ports;sfnt;svg;utils;utils/win;views;views/animated;views/win;xml)
foreach(inc_dir ${inc_dirs})
	list(APPEND skia_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/src/${inc_dir}")
endforeach()
set(skia_include_dirs ${skia_include_dirs} CACHE INTERNAL "")
set(aux_dirs animator;core;device;doc;effects;gpu;lazy;pathops;pdf;utils;sfnt;svg;views)
foreach(aux_dir ${aux_dirs})
	aux_source_directory("src/${aux_dir}" skia_src_${aux_dir})
endforeach()
aux_source_directory(src/utils/win skia_src_utils)
list(REMOVE_ITEM skia_src_utils
		src/utils/SkTaskGroup.cpp
		src/utils/SkLua.cpp
		src/utils/SkLuaCanvas.cpp
		src/utils/SkThreadUtils_pthread.cpp
		src/utils/SkThreadUtils_pthread_linux.cpp
		src/utils/SkThreadUtils_pthread_mach.cpp
		src/utils/SkThreadUtils_pthread_other.cpp
#		src/utils/SkThreadUtils_win.cpp
#		src/utils/win/SkWGL_win.cpp
		)
if (BUILD_SHARED)
	list(REMOVE_ITEM skia_src_utils
			src/utils/SkBitmapHasher.cpp
			src/utils/SkDeferredCanvas.cpp
			)
endif ()
aux_source_directory(src/effects/gradients skia_src_effects)
aux_source_directory(src/gpu/effects skia_src_gpu)
aux_source_directory(src/gpu/gl skia_src_gpu)
list(REMOVE_ITEM skia_src_gpu
		src/gpu/gl/GrGLCreateNativeInterface_none.cpp
		src/gpu/gl/GrGLDefaultInterface_none.cpp
		)
if (BUILD_SHARED)
	list(REMOVE_ITEM skia_src_gpu
			src/gpu/GrLayerHoister.cpp
			src/gpu/SkGpuDevice.cpp
			)
endif ()

#aux_source_directory(src/gpu/gl/angle skia_src_gpu)
aux_source_directory(src/gpu/gl/builders skia_src_gpu)
aux_source_directory(src/gpu/gl/debug skia_src_gpu)
aux_source_directory(src/views/animated skia_src_views)
aux_source_directory(src/views/win skia_src_views)
set(skia_src_fonts
		src/fonts/SkFontMgr_indirect.cpp
		src/fonts/SkGScalerContext.cpp
		src/fonts/SkRemotableFontMgr.cpp
		src/fonts/SkTestScalerContext.cpp
		)
set(skia_src_gpu
		${skia_src_gpu}
		src/gpu/gl/win/GrGLCreateNativeInterface_win.cpp
		src/gpu/gl/win/SkNativeGLContext_win.cpp
		)
set(skia_src_image
		src/image/SkImage.cpp
		src/image/SkImage_Raster.cpp
		src/image/SkImagePriv.cpp
		src/image/SkSurface.cpp
		src/image/SkSurface_Raster.cpp
		)
set(skia_src_images
		src/images/SkImageEncoder.cpp
		src/images/SkImageEncoder_Factory.cpp
		)
if (NOT BUILD_SHARED)
	list(APPEND skia_src_images
			src/images/bmpdecoderhelper.cpp
			src/images/SkDecodingImageGenerator.cpp
			src/images/SkImageDecoder.cpp
			src/images/SkScaledBitmapSampler.cpp
			)
endif ()
set(skia_src_opts
		src/opts/SkBitmapFilter_opts_SSE2.cpp
		src/opts/SkBitmapProcState_opts_SSE2.cpp
		src/opts/SkBitmapProcState_opts_SSSE3.cpp
		src/opts/SkBlitRect_opts_SSE2.cpp
		src/opts/SkBlitRow_opts_SSE2.cpp
		src/opts/SkBlurImage_opts_SSE2.cpp
		src/opts/SkBlurImage_opts_SSE4.cpp
		src/opts/SkMorphology_opts_SSE2.cpp
		src/opts/SkTextureCompression_opts_none.cpp
		src/opts/SkUtils_opts_SSE2.cpp
		src/opts/SkXfermode_opts_SSE2.cpp
		src/opts/opts_check_x86.cpp
		)
set(skia_src_ports
		src/ports/SkDebug_stdio.cpp
#		src/ports/SkDebug_win.cpp
		src/ports/SkDiscardableMemory_none.cpp
		src/ports/SkFontHost_win.cpp
		src/ports/SkFontMgr_default_gdi.cpp
		src/ports/SkGlobalInitialization_default.cpp
		src/ports/SkMemory_malloc.cpp
		src/ports/SkOSFile_stdio.cpp
		src/ports/SkOSFile_win.cpp
		src/ports/SkTime_win.cpp
		src/ports/SkTLS_win.cpp
		#		src/ports/SkTLS_pthread.cpp
		)
set(skia_sources
		${skia_src_gpu}
		${skia_src_device}
		${skia_src_effects}
		${skia_src_fonts}
		${skia_src_image}
		${skia_src_images}
		${skia_src_lazy}
		${skia_src_opts}
		${skia_src_pathops}
		${skia_src_pdf}
		${skia_src_sfnt}
#		${skia_src_svg}
		${skia_src_utils}
		${skia_src_ports}
		${skia_src_core}
		)

#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

source_group("Header Files" FILES ${skia_headers})
source_group("Source Files" FILES ${skia_sources})

if (BUILD_SHARED)
	add_library(skia SHARED ${skia_sources} ${skia_headers})
	find_library(LIB_GL Opengl32.lib OpenGl opengl32 opengl)
	target_link_libraries(skia ${LIB_GL})
	find_library(LIB_USP10 Usp10.lib usp10.lib usp10)
	target_link_libraries(skia ${LIB_USP10})
else()
	add_library(skia STATIC ${skia_sources} ${skia_headers})
endif ()
target_include_directories(skia PUBLIC ${skia_include_dirs})
set_target_properties(skia PROPERTIES 
	VERSION 1.0 
	SOVERSION 1
	FOLDER third_party
	)
	
#find_library(LIB_FONTCONFIG fontconfig)
#find_library(LIB_PNG png)
#find_library(LIB_FREETYPE freetype)
#target_link_libraries(skia ${LIB_FONTCONFIG} ${LIB_PNG})
