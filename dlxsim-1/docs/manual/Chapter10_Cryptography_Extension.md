# Chapter 10: Cryptography Extension (K)

## 10.1 AES Instructions

```assembly
# AES Encryption/Decryption
aesenc      rd, rs1, rs2        # AES single round encrypt
aesenclast  rd, rs1, rs2        # AES final round encrypt
aesdec      rd, rs1, rs2        # AES single round decrypt
aesdeclast  rd, rs1, rs2        # AES final round decrypt
aesimc      rd, rs              # AES inverse mix columns
aeskeygenassist rd, rs, imm     # AES key generation assist

# AES-GCM (Galois/Counter Mode)
aesgcm.enc  vd, vs1, vs2, vs3   # AES-GCM encrypt block
aesgcm.dec  vd, vs1, vs2, vs3   # AES-GCM decrypt block
aesgcm.mac  vd, vs1, vs2        # AES-GCM compute MAC
```

## 10.2 SHA Instructions

```assembly
# SHA-256
sha256.sig0  rd, rs             # SHA-256 Sigma0
sha256.sig1  rd, rs             # SHA-256 Sigma1
sha256.sum0  rd, rs             # SHA-256 Sum0
sha256.sum1  rd, rs             # SHA-256 Sum1
sha256.round rd, rs1, rs2, rs3  # SHA-256 round operation

# SHA-512
sha512.sig0  rd, rs             # SHA-512 Sigma0
sha512.sig1  rd, rs             # SHA-512 Sigma1
sha512.sum0  rd, rs             # SHA-512 Sum0
sha512.sum1  rd, rs             # SHA-512 Sum1
sha512.round rd, rs1, rs2, rs3  # SHA-512 round operation

# SHA-3 (Keccak)
sha3.theta  vd, vs1, vs2        # SHA-3 theta step
sha3.rho    vd, vs              # SHA-3 rho step
sha3.chi    vd, vs              # SHA-3 chi step
sha3.iota   vd, vs, imm         # SHA-3 iota step
```

## 10.3 RSA and ECC

```assembly
# RSA Operations
rsa.modexp  rd, base, exp, mod  # Modular exponentiation
rsa.mulmod  rd, rs1, rs2, mod   # Modular multiplication

# Elliptic Curve Operations
ecc.pointadd  pd, ps1, ps2, curve # EC point addition
ecc.pointmul  pd, ps, scalar, curve # EC scalar multiplication
ecc.verify    rd, msg, sig, pubkey # ECDSA verify
```
