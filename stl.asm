   d01d0:       f3 0f 1e fa             endbr64
   d01d4:       48 8b 47 18             mov    rax,QWORD PTR [rdi+0x18]
   d01d8:       48 85 c0                test   rax,rax
   d01db:       74 13                   je     .L0 -> !right
   d01dd:       0f 1f 00                nop    DWORD PTR [rax]

.L1 -> bst_minimum
   d01e0:       48 89 c2                mov    rdx,rax
   d01e3:       48 8b 40 10             mov    rax,QWORD PTR [rax+0x10]
   d01e7:       48 85 c0                test   rax,rax
   d01ea:       75 f4                   jne    .L1
   d01ec:       48 89 d0                mov    rax,rdx
   d01ef:       c3                      ret


.L0:
   d01f0:       48 8b 57 08             mov    rdx,QWORD PTR [rdi+0x8]
   d01f4:       48 3b 7a 18             cmp    rdi,QWORD PTR [rdx+0x18]
   d01f8:       75 f2                   jne    .L2
   d01fa:       66 0f 1f 44 00 00       nop    WORD PTR [rax+rax*1+0x0]
.L3
   d0200:       48 89 d0                mov    rax,rdx
   d0203:       48 8b 52 08             mov    rdx,QWORD PTR [rdx+0x8]
   d0207:       48 39 42 18             cmp    QWORD PTR [rdx+0x18],rax
   d020b:       74 f3                   je     .L3
   d020d:       48 3b 50 18             cmp    rdx,QWORD PTR [rax+0x18]
   d0211:       48 0f 44 d0             cmove  rdx,rax
   d0215:       48 89 d0                mov    rax,rdx
   d0218:       c3                      ret
   d0219:       0f 1f 80 00 00 00 00    nop    DWORD PTR [rax+0x0]
.L2
   d01ec:       48 89 d0                mov    rax,rdx
   d01ef:       c3                      ret
