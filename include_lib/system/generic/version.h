#ifndef VERSION_H
#define VERSION_H

#include "typedef.h"


typedef int (*version_t)(int);


//定义模块的版本号，由主版本号和次版本号组成
//如果两个模块的主版本号相同即表示兼容
#define VERSION(major, minor) 		(((major)<<8) | (minor))

#define MAJOR(v)    ((v) >> 16)
#define MINOR(v)    (((v) >> 8) & 0xff)

#define version_match(module_a, module_b) \
	({ \
	 	extern int module_a##_version(int ); \
	 	extern int module_b##_version(int ); \
	 	int version_a = module_a##_version(0); \
	 	int version_b = module_b##_version(0); \
		MAJOR(version_a) == MAJOR(version_b);\
	 })





#define __MODULE_VERSION_EXPORT_BEGIN(module, version) \
	int  module##_version(int prt) \
	{ \
        if (prt) { \
            log_i(#module": %d.%d.%d  build at: %s\n", (version)>>16, \
                    ((version) >> 8) & 0xff, (version) & 0xff, __DATE__); \
        } \

#define __MODULE_VERSION_EXPORT_END(module, version) \
		return version; \
	} \
	const version_t __version_##module  \
        __attribute__((section(".lib_version"),used)) = module##_version

#define __MODULE_VERSION_EXPORT(module, version) \
    __MODULE_VERSION_EXPORT_BEGIN(module, version) \
    __MODULE_VERSION_EXPORT_END(module, version);


#define __MODULE_VERSION_EXPORT_SECTION(module, version, section) \
    __MODULE_VERSION_EXPORT_BEGIN(module, version) \
        (void *)&section; \
    __MODULE_VERSION_EXPORT_END(module, version)

#define __MODULE_DEPEND_BEGIN(module) \
	int module##_version_check() \
	{ \


#define _MODULE_DEPEND_BEGIN(module) \
	__MODULE_DEPEND_BEGIN(module)


#define __VERSION_CHECK(module, version) \
	do { \
        int  module##_version(int prt); \
        int v = module##_version(0);  \
        if (MAJOR(version) != MAJOR(v) || MINOR(version) > MINOR(v)) { \
            log_i("=======version not match=======\n");  \
            module##_version(1); \
            log_i("==================================\n"); \
            while(1); \
        } \
	} while(0)
/*-------------------上面的宏请勿调用------------------------------------*/






//定义当前模块的版本检测函数
#define MODULE_VERSION_EXPORT(module, version) \
	__MODULE_VERSION_EXPORT(module, version)

#define MODULE_VERSION_EXPORT_SECTION(module, version, section) \
	__MODULE_VERSION_EXPORT_SECTION(module, version, section)

#define MODULE_VERSION_EXPORT_BEGIN(module, version) \
	__MODULE_VERSION_EXPORT_BEGIN(module, version)

#define MODULE_VERSION_EXPORT_END(module, version) \
	__MODULE_VERSION_EXPORT_END(module, version)

//以下3个宏定义当前模块依赖的其它模块列表
#define MODULE_DEPEND_BEGIN() \
	_MODULE_DEPEND_BEGIN(THIS_MODULE)

#define MODULE_DEPEND(module_d, version) \
        __VERSION_CHECK(module_d, version)

#define MODULE_DEPEND_END() \
		return 0; \
	}


#define VERSION_CHECK(module, version) \
    __VERSION_CHECK(module, version)


//通过调用版本检测函数使的模块的代码能够被链接
#define  load_module(module) \
	({ \
	 	int ret; \
		extern int module##_version_check(); \
		ret = module##_version_check();\
	 	ret; \
	})

extern version_t lib_version_begin[], lib_version_end[];

#define lib_version_check() \
    do { \
        version_t *version; \
        log_i("=========version check===========\n"); \
        for (version = lib_version_begin; version < lib_version_end; version++) { \
            (*version)(1); \
        }; \
        log_i("==================================\n\n"); \
    } while (0)


#endif

