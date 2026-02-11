# Chapter 3: Building

## 3.1 Prerequisites

- C compiler (gcc, clang, or cc)
- Make
- Standard C library

## 3.2 Compilation

```bash
cd dlxsim-1
make
```

This produces the `dlxsim` executable.

## 3.3 Installation

### System-wide Installation

```bash
make install DSTROOT=/
```

### Staging Directory Installation

```bash
make install DSTROOT=/tmp/staging
```

The `DSTROOT` variable specifies the destination root for installation.

## 3.4 Build Targets

```bash
make                # Build dlxsim
make clean          # Remove build artifacts
make install        # Install to DSTROOT
```

## 3.5 Compiler Options

The Makefile uses standard compiler flags:
- `-O2` for optimization
- `-Wall` for warnings
- `-g` for debug symbols (optional)
