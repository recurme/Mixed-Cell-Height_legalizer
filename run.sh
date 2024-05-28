#!/bin/bash
# benchmark=(des_perf_1 des_perf_a_md1  des_perf_b_md1  edit_dist_1_md1  edit_dist_a_md3  fft_a_md2  pci_bridge32_a_md1  pci_bridge32_b_md1  pci_bridge32_b_md3
# des_perf_a_md2  des_perf_b_md2  edit_dist_a_md2  fft_2_md2        fft_a_md3  pci_bridge32_a_md2  pci_bridge32_b_md2)
benchmark="des_perf_1"
./build/MCHLG -doParallel true -lef ./benchmarks/$benchmark/tech.lef -lef ./benchmarks/$benchmark/cells_modified.lef -def ./benchmarks/$benchmark/placed.def -placement_constraints ./benchmarks/$benchmark/placement.constraints -output_def ./output/$benchmark.def