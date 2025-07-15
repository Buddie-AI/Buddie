#ifndef SYS_INIT_H
#define SYS_INIT_H







typedef int (*initcall_t)(void);

#define __initcall(fn)  \
	const initcall_t __initcall_##fn sec(.initcall) = fn

#define early_initcall(fn)  \
	const initcall_t __initcall_##fn sec(.early.initcall) = fn


#define late_initcall(fn)  \
	const initcall_t __initcall_##fn sec(.late.initcall) = fn


#define platform_initcall(fn) \
	const initcall_t __initcall_##fn sec(.platform.initcall) = fn


#define module_initcall(fn) \
	const initcall_t __initcall_##fn sec(.module.initcall) = fn




#define __do_initcall(prefix) \
	do { \
		initcall_t *init; \
		extern initcall_t prefix##_begin[], prefix##_end[]; \
		for (init=prefix##_begin; init<prefix##_end; init++) { \
			(*init)();	\
		} \
	}while(0)







#endif

