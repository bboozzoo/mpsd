#ifndef __PLUGIN_H__
#define __PLUGIN_H__

/**
 * @brief structure describing plugin
 */
struct plugin_s {
    void (*init) (void);
    void (*finalize) (void);
};

#endif /* __PLUGIN_H__ */
