#ifndef __COBALT_COBALT_CONFIG_H_
#define __COBALT_COBALT_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_COBALT_DIRECTORY_PATH "/sdcard/Cobalt"
#define CONFIG_FILE_PATH "/config.json"
#define LOG_FILE_PATH "/latest.log"
    
void config_refresh();
int config_get_int(char* name);
char* config_get_string(char* name);
void config_cleanup();

#ifdef __cplusplus
}
#endif

#endif // __COBALT_COBALT_CONFIG_H_
