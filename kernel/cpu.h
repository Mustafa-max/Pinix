#ifndef CPU_H
#define CPU_H

typedef struct {
    unsigned int vendor_id[4];
    unsigned int family;
    unsigned int model;
    unsigned int stepping;
    unsigned int features_edx;
    unsigned int features_ecx;
} cpu_info_t;

void cpu_init(void);
cpu_info_t* cpu_get_info(void);

// Intel ve AMD işlemci özel fonksiyon bildirimleri
void intel_cpu_init(void);
void intel_configure_features(void);
void amd_cpu_init(void);
void amd_configure_features(void);

#endif