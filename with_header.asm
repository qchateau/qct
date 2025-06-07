   16380:       f3 0f 1e fa             endbr64
   16384:       48 8b 47 10             mov    rax,QWORD PTR [rdi+0x10]
   16388:       48 85 c0                test   rax,rax
   1638b:       74 13                   je     .L0
   1638d:       0f 1f 00                nop    DWORD PTR [rax]

.L1:
   16390:       49 89 c0                mov    r8,rax
   16393:       48 8b 40 08             mov    rax,QWORD PTR [rax+0x8]
   16397:       48 85 c0                test   rax,rax
   1639a:       75 f4                   jne    .L1
   1639c:       4c 89 c0                mov    rax,r8
   1639f:       c3                      ret

.L0:
   163a0:       4c 8b 07                mov    r8,QWORD PTR [rdi]
   163a3:       49 3b 78 10             cmp    rdi,QWORD PTR [r8+0x10]
   163a7:       75 f3                   jne    .L2
   163a9:       0f 1f 80 00 00 00 00    nop    DWORD PTR [rax+0x0]
.L3:
   163b0:       4c 89 c0                mov    rax,r8
   163b3:       4d 8b 00                mov    r8,QWORD PTR [r8]
   163b6:       49 39 40 10             cmp    QWORD PTR [r8+0x10],rax
   163ba:       74 f4                   je     .L3
   163bc:       4c 3b 40 10             cmp    r8,QWORD PTR [rax+0x10]
   163c0:       4c 0f 44 c0             cmove  r8,rax
   163c4:       4c 89 c0                mov    rax,r8
   163c7:       c3                      ret
   163c8:       0f 1f 84 00 00 00 00    nop    DWORD PTR [rax+rax*1+0x0]

.L2:
   1639c:       4c 89 c0                mov    rax,r8
   1639f:       c3                      ret
