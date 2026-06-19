#ifndef QTAT_EXPORT_H
#define QTAT_EXPORT_H

#if defined(QTAT_BUILD_STATIC)
    #define QTAUTOTEST_EXPORT
#elif defined(QTAT_BUILD_SHARED)
    #if defined(_WIN32) || defined(__CYGWIN__)
        #ifdef QtAutoTestServer_EXPORTS
            #define QTAUTOTEST_EXPORT __declspec(dllexport)
        #else
            #define QTAUTOTEST_EXPORT __declspec(dllimport)
        #endif
    #else
        #define QTAUTOTEST_EXPORT __attribute__((visibility("default")))
    #endif
#else
    #define QTAUTOTEST_EXPORT
#endif

#endif // QTAT_EXPORT_H
