#ifndef INIT_H
#define INIT_H

#ifdef __cplusplus
extern "C" {
#endif

  bool app_init_libraries(void);
  bool sensor_init_libraries(void);
  bool app_finalize_libraries(void);

#ifdef __cplusplus
}
#endif

#endif // FOO_H
