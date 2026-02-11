# Chapter 20: Quantum Computing Interface (Q Extension)

## 20.1 Quantum Gates

```assembly
# Single-Qubit Gates
qh      qubit_id                # Hadamard gate
qx      qubit_id                # Pauli-X (NOT) gate
qy      qubit_id                # Pauli-Y gate
qz      qubit_id                # Pauli-Z gate
qs      qubit_id                # S gate (phase)
qt      qubit_id                # T gate (π/8 phase)

# Rotation Gates
qrx     qubit_id, angle         # Rotate around X axis
qry     qubit_id, angle         # Rotate around Y axis
qrz     qubit_id, angle         # Rotate around Z axis

# Two-Qubit Gates
qcnot   control, target         # Controlled-NOT (CNOT)
qcz     control, target         # Controlled-Z
qswap   qubit1, qubit2          # SWAP gate

# Three-Qubit Gates
qtoffoli ctrl1, ctrl2, target   # Toffoli (CCNOT) gate
qfredkin ctrl, target1, target2 # Fredkin (CSWAP) gate
```

## 20.2 Quantum Measurement

```assembly
qmeasure    rd, qubit_id        # Measure qubit: rd ← {0, 1}
qmeasure.all vd, qubit_mask     # Measure multiple qubits
qreset      qubit_id            # Reset qubit to |0⟩
```
