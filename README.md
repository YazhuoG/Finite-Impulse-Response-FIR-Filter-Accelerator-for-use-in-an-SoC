# FIR Filter Accelerator for SoC

## Project Overview

This project implements a hardware-accelerated Finite Impulse Response (FIR) filter integrated into a Rocket Chip System-on-Chip (SoC) platform.  
The goal is to improve signal processing throughput by offloading computation from the CPU to a dedicated hardware accelerator while maintaining area efficiency and functional correctness.

Key objectives:
- Hardware-software co-design for FIR computation  
- Performance improvement compared to software-only implementation  
- Efficient memory access and pipeline optimization  

---

## System Architecture

The accelerator consists of the following components:

- DMA-based data transfer for input and coefficient loading  
- Sliding window buffer to reuse input data efficiently  
- Pipelined FIR computation core  
- FIFO interface for output data  
- Control logic to switch between FIR operations  

The Rocket CPU configures the accelerator, transfers data, and retrieves computation results.

![Architecture](FIR_Filter_Diagram.png)
Figure: FIR accelerator dataflow.

Input samples are transferred from main memory via DMA and buffered in a sliding window.
The pipelined FIR core processes data continuously and writes results to a FIFO buffer.
The Rocket CPU configures the accelerator and retrieves results through memory-mapped registers.

---

## Performance Results

The hardware accelerator was evaluated using Rocket simulation and VSIM environments.

| Metric | Result |
|--------|--------|
| Rocket_sim Speedup | 118× |
| VSIM Speedup | 26.9× |
| Throughput | 4 results per cycle |
| Critical Path Delay | 2.37 ns |

The accelerator significantly reduces execution time by offloading computation and applying pipeline optimization.

---

## Design Highlights

Key engineering decisions and optimizations:

- Pipeline optimization of the FIR computation loop to increase throughput  
- DMA-based memory access to reduce CPU overhead  
- Sliding window buffering to reduce redundant memory access  
- FIFO-based output handling to support continuous data processing  
- Hardware-software partitioning to balance flexibility and performance  

---

## Verification and Testing

Verification steps included:

- Functional simulation to validate FIR correctness  
- Comparison with software-only implementation  
- Performance benchmarking  
- Error checking to ensure zero computation error  

---

## Implementation Details

Key techniques used:

- Hardware-software co-simulation  
- FIFO management and stall avoidance  
- Loop restructuring to satisfy scheduling constraints  
- Buffer management for streaming computation  

---

## Repository Structure

/hls                            (Data transfer tools)

/rocket_sim                     (Rokect Chip simulation setup)

/sc                             (Hardware and software source files ) 

/vsim                           (Simulation scripts and configurations)

ECE 720 Project2 Report.pdf     (Project report and documentation)

---

## Skills Demonstrated

- SoC integration  
- Hardware acceleration  
- Performance optimization  
- DMA and memory access optimization  
- Pipeline design  
- Benchmarking and profiling  

This project demonstrates practical accelerator design and performance analysis in a SoC environment.
