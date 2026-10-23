/*
 * vmpinix CPU Subsystem - topology.c
 * Processor Topology, Core Mapping, and APIC ID Enumeration Implementation
 */

#include "../kernel/cpu.h"

#define MAX_CPU_TOPOLOGY_NODES 256

typedef struct {
    unsigned int apic_id;
    unsigned int package_id;
    unsigned int core_id;
    unsigned int thread_id;
    int active;
} cpu_topology_node_t;

static cpu_topology_node_t topology_map[MAX_CPU_TOPOLOGY_NODES];
static unsigned int topology_node_count = 0;

static void cpuid_topo(unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
}

int cpu_topology_init(void) {
    topology_node_count = 0;

    for (int i = 0; i < MAX_CPU_TOPOLOGY_NODES; i++) {
        topology_map[i].apic_id = 0;
        topology_map[i].package_id = 0;
        topology_map[i].core_id = 0;
        topology_map[i].thread_id = 0;
        topology_map[i].active = 0;
    }

    // İlk çekirdek (BSP) kaydı
    unsigned int eax, ebx, ecx, edx;
    cpuid_topo(1, 0, &eax, &ebx, &ecx, &edx);

    unsigned int bsp_apic_id = (ebx >> 24) & 0xFF;

    topology_map[0].apic_id = bsp_apic_id;
    topology_map[0].package_id = 0;
    topology_map[0].core_id = 0;
    topology_map[0].thread_id = 0;
    topology_map[0].active = 1;
    topology_node_count = 1;

    return 0;
}

int cpu_topology_register_core(unsigned int apic_id, unsigned int package, unsigned int core, unsigned int thread) {
    if (topology_node_count >= MAX_CPU_TOPOLOGY_NODES) {
        return -1;
    }

    for (unsigned int i = 0; i < topology_node_count; i++) {
        if (topology_map[i].apic_id == apic_id) {
            return 0; // Zaten kayıtlı
        }
    }

    cpu_topology_node_t *node = &topology_map[topology_node_count++];
    node->apic_id = apic_id;
    node->package_id = package;
    node->core_id = core;
    node->thread_id = thread;
    node->active = 1;

    return 0;
}

cpu_topology_node_t* cpu_topology_get_node(unsigned int index) {
    if (index >= topology_node_count) {
        return (cpu_topology_node_t*)0;
    }
    return &topology_map[index];
}

unsigned int cpu_topology_get_count(void) {
    return topology_node_count;
}