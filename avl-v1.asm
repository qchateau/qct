    acd0:       f3 0f 1e fa             endbr64
    acd4:       48 8b 47 10             mov    rax,QWORD PTR [rdi+0x10]
    acd8:       48 85 c0                test   rax,rax
    acdb:       74 13                   je     .L0 -> !right
    acdd:       0f 1f 00                nop    DWORD PTR [rax]

.L1 -> bst_minimum
    ace0:       49 89 c0                mov    r8,rax
    ace3:       48 8b 40 08             mov    rax,QWORD PTR [rax+0x8]
    ace7:       48 85 c0                test   rax,rax
    acea:       75 f4                   jne    .L1
    acec:       4c 89 c0                mov    rax,r8
    acef:       c3                      ret

.L0
    acf0:       4c 8b 07                mov    r8,QWORD PTR [rdi]
    acf3:       4d 85 c0                test   r8,r8
    acf6:       75 16                   jne    .L2
    acf8:       eb f2                   jmp    .L3
    acfa:       66 0f 1f 44 00 00       nop    WORD PTR [rax+rax*1+0x0]

.L5
    ad00:       49 8b 00                mov    rax,QWORD PTR [r8]
    ad03:       4c 89 c7                mov    rdi,r8
    ad06:       48 85 c0                test   rax,rax
    ad09:       74 15                   je     .L4
    ad0b:       49 89 c0                mov    r8,rax

.L2
    ad0e:       49 3b 78 10             cmp    rdi,QWORD PTR [r8+0x10]
    ad12:       74 ec                   je     .L5
    ad14:       eb d6                   jmp    .L3

.L3
    acec:       4c 89 c0                mov    rax,r8
    acef:       c3                      ret
    ad16:       66 2e 0f 1f 84 00 00    cs nop WORD PTR [rax+rax*1+0x0]
    ad1d:       00 00 00
.L4
    ad20:       45 31 c0                xor    r8d,r8d
    ad23:       4c 89 c0                mov    rax,r8
    ad26:       c3                      ret
    ad27:       66 0f 1f 84 00 00 00    nop    WORD PTR [rax+rax*1+0x0]
