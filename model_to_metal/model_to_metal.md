# model-to-metal

A learning project. You build a working ML stack end to end, **model → compiler → accelerator simulator → RTL**, starting deliberately crude, then make every layer better on each pass. What matters is what you learn while writing the code, not the code itself.

## Goals
- Understand how a neural network actually runs on custom silicon, from the training math down to the hardware.
- Close the gap between *using* a technology and *understanding* it, at every layer.
- Learn by building: each layer is a real, testable component, checked against the layer above it.

## Approach

### Breadth-first passes
Each pass raises **every** area about one level before any area goes deeper. The project is rebuilt or extended on each pass. It is never finished one layer at a time.

| Pass | Target depth | What gets built |
|---|---|---|
| **1: map of the territory** | ~L2 everywhere: the concepts, and a crude working version | MLP → hand-written compiler → toy ISA → functional simulator → MAC unit in RTL |
| **2: working knowledge** | ~L3 everywhere: can work in the area independently, with reference | Transformer → tiling compiler (MLIR optional) → systolic-array accelerator (simulator + RTL) → UVM testbench, AXI-lite |
| **3+: depth** | Chosen per area | Driven by interest and need |

### Rules for using AI
The project is meant to be built *with* an AI assistant, under rules that protect the learning:

1. **If it's the lesson, you write it. If it's plumbing, AI writes it.** Every milestone says which is which. Plumbing means build files, test scaffolding, CLI parsing, data generation, plotting, and tool setup.
2. **Tutor by default.** The AI doesn't write lesson code unless you explicitly override it (for example, "just write it"). When you're stuck, it climbs a hint ladder, one rung at a time: concept → direction → pseudocode → small snippet. Try for ~15–20 minutes on your own first.
3. **Reviewer, not fixer.** The AI reviews your code like a strict senior engineer and explains problems without fixing them.
4. **Predict before running.** Before anything instructive runs, write down what you expect: output, `sizeof`, which destructor runs, performance. Wrong predictions are where you learn the most.
5. **Compare after.** Once your version works, the AI writes its own and you study the differences.
6. **Bookend each session.** Start with 2–3 recall questions, from any earlier area. End by checking off progress and logging what was hard.

## Architecture (pass 1)
```
 MLP (C++ reference) ──► compiler ──► toy ISA program ──► functional simulator
        ▲                                                      │ same answer?
        └──────────────────────── compare ◄────────────────────┘
                                          MAC unit RTL ◄── compare ── simulator MAC op
```
Every layer has an *oracle*: the layer above it. The C++ model checks the simulator, and the simulator checks the RTL.

## Repository layout
```
model-to-metal/
├── MODULE.bazel, .bazelversion, .bazelrc, defs.bzl
├── tensor/      # M1: owning 2-D tensor type
├── kernels/     # M2: matmul variants, benchmarks
├── nn/          # M3: layers, loss, backprop, training
├── weights/     # M4: binary format + mmap loader
├── sim/         # M5: ISA spec, decoder, simulator (static library)
├── compiler/    # M6: graph IR, lowering, codegen
├── analysis/    # M7: FLOP/byte counting, roofline
├── rtl/         # M8: SystemVerilog + testbench
└── tools/       # Python plotting / data scripts (AI-written)
```

## Toolchain
- **C++20** with clang or gcc.
- **Bazel** via Bazelisk, using bzlmod and `rules_cc`. A shared `defs.bzl` wrapper applies the same warnings to every target and links GoogleTest into tests.
- **Configs:** debug by default (so the debugger always works), `opt` for benchmarks, and `asan`, `ubsan`, `tsan` for the sanitizers.
- **HDL simulator:** Verilator or Icarus Verilog, added at M8.
- **Python 3**, for plots only.

## Pass 1 milestones
| # | Milestone | Areas | You write | AI writes | Done when |
|---|---|---|---|---|---|
| **M0** | Setup | Build systems | n/a | Bazel module, configs, wrappers, a sanity test | `bazel test //...` passes in every config |
| **M1** | Tensor type | C++ memory layout, references vs pointers, ownership/RAII, move semantics, rule of 0/3/5 | An owning 2-D `Tensor`: first a raw `new[]`/`delete[]` version (rule of 5), then a rule-of-0 refactor | Tests, printing helpers | Clean under ASan/UBSan, and you can explain every special member function |
| **M2** | Matmul + memory hierarchy | Caches, locality, threads, races, false sharing | Naive, loop-reordered, blocked, and multithreaded matmul; a deliberate data race and a false-sharing demo | Benchmark harness, plots | Every performance result explained in terms of cache lines |
| **M3** | MLP + backprop | ML fundamentals, linear algebra, calculus | Linear layer, ReLU, softmax + cross-entropy, backward pass, SGD, training loop (derive the gradients on paper first) | Spiral dataset, decision-boundary plot, numerical gradient checker | Gradient check passes; >95% accuracy on spirals |
| **M4** | Weights via `mmap` | Virtual memory, page cache, binary layout, endianness, alignment | File format and `mmap` loader | Save side (optional) | Reloaded model gives identical outputs |
| **M5** | Toy ISA + simulator | ISA design, simulator design, linking | ISA spec (`LOAD`/`STORE`/`MATMUL`/`ADD`/`RELU`/`HALT`, memory model, encoding), decoder, execute loop, memory model, built as a static library | Assembler/disassembler, test programs | Hand-written programs run correctly; `nm` exercise on the library |
| **M6** | Compiler | IR, lowering, buffer allocation, codegen | Graph IR, lowering to the toy ISA, scratchpad allocation, code emission | IR pretty-printer, test scaffolding | Compiled MLP on the simulator matches M3 within tolerance |
| **M7** | Roofline analysis | FLOPs, bytes moved, arithmetic intensity | Per-op counters in the simulator, roofline analysis | Roofline plot | Can explain why each op is compute- or memory-bound, and what would change that |
| **M8** | MAC unit in RTL | Combinational vs sequential logic, valid/ready, FIFOs, testbench structure | MAC unit with valid/ready and an input FIFO | Tool setup, testbench scaffolding, a walkthrough mapping it onto UVM | Random stimulus matches the simulator, including random backpressure |
| **M9** | End to end | Everything | Train → save → compile → simulate → compare; retrospective | n/a | Full chain matches; pass 2 priorities written down |

**Pass 1 side sessions**, for topics the project doesn't reach: RISC-V privileged basics (traps, CSRs), 5-stage pipeline and hazards (on paper), syscalls, and attention (concept only, ahead of pass 2).

## Pass 2 preview
- **Model:** a small transformer (attention, layer norm, KV cache), checked against a known reference implementation.
- **Kernels:** tiling, int8 quantization, operator fusion, and a FlashAttention-style attention kernel.
- **Accelerator:** a systolic array with on-chip SRAM and double-buffered DMA, in both the simulator and the RTL.
- **Compiler:** tiling and scheduling passes. Optionally rebuilt on MLIR.
- **RTL and verification:** a systolic array in SystemVerilog, an AXI-lite control interface, a UVM testbench written by you, and SVA assertions.
- **Systems:** C++ memory ordering and lock-free basics, out-of-order execution and cache coherence concepts applied in the simulator.

## Principles
- **Crude first, correct always.** Every layer is simple but checked against its oracle.
- **Wrong predictions are progress.** Log them.
- **No timelines.** Depth comes from repeated passes, not from finishing any one layer early.
