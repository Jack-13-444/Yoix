extern interrupt_handler
global isr_stub_table


isr_stub_table:
%assign i 0 
%rep    256 
    DQ isr_stub_%+i ; use DQ instead if targeting 64-bit
    
%assign i i+1 
%endrep


%macro isr_err_stub 1
isr_stub_%+%1:
    push %1             ; push interrupt number
    jmp isr_common
%endmacro
; if writing for 64-bit, use iretq instead
%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0 
    push %1

    jmp isr_common

%endmacro

%macro isr_Reserved_stub 1
isr_stub_%+%1:

    iretq
%endmacro

isr_common:

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rax, rax
    mov ax, ds
    push rax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rdi, rsp 
    
    call interrupt_handler

    pop rax

    mov ds, ax ; restore old segments
    mov es, ax
    mov fs, ax
    mov gs, ax
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rbx
    pop rax

    add rsp, 16
    iretq


isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

isr_no_err_stub 32 ; for timer
isr_no_err_stub 33
isr_no_err_stub 34
isr_no_err_stub 35
isr_no_err_stub 36
isr_no_err_stub 37
isr_no_err_stub 38
isr_no_err_stub 39
isr_no_err_stub 40
isr_no_err_stub 41
isr_no_err_stub 42
isr_no_err_stub 43

isr_no_err_stub 44
isr_no_err_stub 45
isr_no_err_stub 46
isr_no_err_stub 47
isr_Reserved_stub 48
isr_Reserved_stub 49
isr_Reserved_stub 50
isr_Reserved_stub 51
isr_Reserved_stub 52
isr_Reserved_stub 53
isr_Reserved_stub 54
isr_Reserved_stub 55
isr_Reserved_stub 56
isr_Reserved_stub 57
isr_Reserved_stub 58
isr_Reserved_stub 59
isr_Reserved_stub 60
isr_Reserved_stub 61
isr_Reserved_stub 62
isr_Reserved_stub 63
isr_Reserved_stub 64
isr_Reserved_stub 65
isr_Reserved_stub 66
isr_Reserved_stub 67

isr_Reserved_stub 68
isr_Reserved_stub 69
isr_Reserved_stub 70
isr_Reserved_stub 71
isr_Reserved_stub 72
isr_Reserved_stub 73
isr_Reserved_stub 74
isr_Reserved_stub 75
isr_Reserved_stub 76
isr_Reserved_stub 77
isr_Reserved_stub 78
isr_Reserved_stub 79

isr_Reserved_stub 80
isr_Reserved_stub 81
isr_Reserved_stub 82
isr_Reserved_stub 83
isr_Reserved_stub 84
isr_Reserved_stub 85
isr_Reserved_stub 86
isr_Reserved_stub 87
isr_Reserved_stub 88
isr_Reserved_stub 89
isr_Reserved_stub 90
isr_Reserved_stub 91

isr_Reserved_stub 92
isr_Reserved_stub 93
isr_Reserved_stub 94
isr_Reserved_stub 95
isr_Reserved_stub 96
isr_Reserved_stub 97
isr_Reserved_stub 98
isr_Reserved_stub 99
isr_Reserved_stub 100
isr_Reserved_stub 101
isr_Reserved_stub 102
isr_Reserved_stub 103
isr_Reserved_stub 104
isr_Reserved_stub 105
isr_Reserved_stub 106
isr_Reserved_stub 107
isr_Reserved_stub 108
isr_Reserved_stub 109
isr_Reserved_stub 110
isr_Reserved_stub 111
isr_Reserved_stub 112
isr_Reserved_stub 113
isr_Reserved_stub 114
isr_Reserved_stub 115

isr_Reserved_stub 116
isr_Reserved_stub 117
isr_Reserved_stub 118
isr_Reserved_stub 119
isr_Reserved_stub 120
isr_Reserved_stub 121
isr_Reserved_stub 122
isr_Reserved_stub 123
isr_Reserved_stub 124
isr_Reserved_stub 125
isr_Reserved_stub 126
isr_Reserved_stub 127
isr_Reserved_stub 128
isr_Reserved_stub 129
isr_Reserved_stub 130
isr_Reserved_stub 131
isr_Reserved_stub 132
isr_Reserved_stub 133
isr_Reserved_stub 134
isr_Reserved_stub 135
isr_Reserved_stub 136
isr_Reserved_stub 137
isr_Reserved_stub 138
isr_Reserved_stub 139
isr_Reserved_stub 140
isr_Reserved_stub 141
isr_Reserved_stub 142
isr_Reserved_stub 143

isr_Reserved_stub 144
isr_Reserved_stub 145
isr_Reserved_stub 146
isr_Reserved_stub 147
isr_Reserved_stub 148
isr_Reserved_stub 149
isr_Reserved_stub 150
isr_Reserved_stub 151
isr_Reserved_stub 152
isr_Reserved_stub 153
isr_Reserved_stub 154
isr_Reserved_stub 155


isr_Reserved_stub 156
isr_Reserved_stub 157
isr_Reserved_stub 158
isr_Reserved_stub 159
isr_Reserved_stub 160
isr_Reserved_stub 161
isr_Reserved_stub 162
isr_Reserved_stub 163
isr_Reserved_stub 164
isr_Reserved_stub 165
isr_Reserved_stub 166
isr_Reserved_stub 167
isr_Reserved_stub 168
isr_Reserved_stub 169
isr_Reserved_stub 170
isr_Reserved_stub 171

isr_Reserved_stub 172
isr_Reserved_stub 173
isr_Reserved_stub 174
isr_Reserved_stub 175
isr_Reserved_stub 176
isr_Reserved_stub 177
isr_Reserved_stub 178
isr_Reserved_stub 179
isr_Reserved_stub 180
isr_Reserved_stub 181
isr_Reserved_stub 182
isr_Reserved_stub 183

isr_Reserved_stub 184
isr_Reserved_stub 185
isr_Reserved_stub 186
isr_Reserved_stub 187
isr_Reserved_stub 188
isr_Reserved_stub 189
isr_Reserved_stub 190
isr_Reserved_stub 191
isr_Reserved_stub 192
isr_Reserved_stub 193
isr_Reserved_stub 194
isr_Reserved_stub 195
isr_Reserved_stub 196
isr_Reserved_stub 197
isr_Reserved_stub 198
isr_Reserved_stub 199

isr_Reserved_stub 200
isr_Reserved_stub 201
isr_Reserved_stub 202
isr_Reserved_stub 203
isr_Reserved_stub 204
isr_Reserved_stub 205
isr_Reserved_stub 206
isr_Reserved_stub 207
isr_Reserved_stub 208
isr_Reserved_stub 209
isr_Reserved_stub 210
isr_Reserved_stub 211


isr_Reserved_stub 212
isr_Reserved_stub 213
isr_Reserved_stub 214
isr_Reserved_stub 215
isr_Reserved_stub 216
isr_Reserved_stub 217
isr_Reserved_stub 218
isr_Reserved_stub 219
isr_Reserved_stub 220
isr_Reserved_stub 221
isr_Reserved_stub 222
isr_Reserved_stub 223
isr_Reserved_stub 224
isr_Reserved_stub 225
isr_Reserved_stub 226
isr_Reserved_stub 227

isr_Reserved_stub 228
isr_Reserved_stub 229
isr_Reserved_stub 230
isr_Reserved_stub 231
isr_Reserved_stub 232
isr_Reserved_stub 233
isr_Reserved_stub 234
isr_Reserved_stub 235
isr_Reserved_stub 236
isr_Reserved_stub 237
isr_Reserved_stub 238
isr_Reserved_stub 239


isr_Reserved_stub 240
isr_Reserved_stub 241
isr_Reserved_stub 242
isr_Reserved_stub 243
isr_Reserved_stub 244
isr_Reserved_stub 245
isr_Reserved_stub 246
isr_Reserved_stub 247
isr_Reserved_stub 248
isr_Reserved_stub 249
isr_Reserved_stub 250
isr_Reserved_stub 251
isr_Reserved_stub 252
isr_Reserved_stub 253
isr_Reserved_stub 254
isr_Reserved_stub 255 ; svr


section .note.GNU-stack noalloc noexec nowrite progbits