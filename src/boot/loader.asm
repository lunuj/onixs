[org 0x1000]

dw 0x55AA; 魔数，用于判断错误

call loading
call detect_memory
call prepare_protected_mode

detect_memory:
    xor ebx, ebx
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer
    mov edx, 0x534d4150
.next:
    mov eax, 0xE820
    mov ecx, 20
    int 0x15
    jc error
    add di, cx
    inc dword [ards_count]
    cmp ebx, 0
    jnz .next
    call detecing
.done:
    ret

prepare_protected_mode:
    cli
    ;打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al
    
    lgdt [gdt_ptr]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword code_selector:protect_mode

memory_base equ 0
memory_limit equ (1024 * 1024 * 1024 * 4) /(1024 * 4) -1
code_selector equ (1 << 3)
data_selector equ (2 << 3)
gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base
gdt_base:
    dd 0, 0; NULL 描述符
gdt_code:
    dw memory_limit & 0xFFFF
    dw memory_base & 0xFFFF
    db memory_base >> 16 & 0xFF
    db 0b_1_00_1_1_0_1_0
    db 0b_1_1_0_0_0000 | (memory_limit >> 16)&0xF
    db (memory_base >> 24) & 0xFF
gdt_data:
    dw memory_limit & 0xFFFF
    dw memory_base & 0xFFFF
    db memory_base >> 16 & 0xFF
    db 0b_1_00_1_0_0_1_0
    db 0b_1_1_0_0_0000 | (memory_limit >> 16)&0xF
    db (memory_base >> 24) & 0xFF
gdt_end:
string_loading:
        db "Loading Onix...", 10, 13, 0
string_detecting:
        db "Detecting Memory Success...", 10, 13, 0
string_error:
        db "Loading Error...", 10, 13, 0
loading:
    mov si, string_loading
    call print
    ret
detecing:
    mov si, string_detecting
    call print
    ret
error:
    mov si, string_error
    call print
    hlt
    jmp $

print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done

    int 0x10
    inc si
    jmp .next
.done:
    ret
[bits 32]
protect_mode:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x10000

    mov edi, 0x10000
    mov ecx, 10
    mov bl, 200

    call read_disk

    mov eax, 0xAA5555AA
    mov ebx, ards_count

    jmp dword code_selector:0x10040

    ud2

read_disk:
    ; 设置读写扇区数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx; 0x1f3 
    mov al, cl; 起始扇区0~7位
    out dx, al

    inc dx; 0x1f4
    shr ecx, 8; 起始扇区8~15位
    mov al, cl
    out dx, al

    inc dx; 0x1f5 
    shr ecx, 8; 起始扇区16~23位
    mov al, cl
    out dx, al
    
    inc dx; 0x1f6
    shr ecx, 8; 起始扇区23~27位
    and cl, 0b1111

    mov al, 0b1110_0000
    or al, cl
    out dx, al

    inc dx; 0x1f7
    mov al, 0x20
    out dx, al
    xor ecx, ecx
    mov cl, bl
    .read_block:
        push cx
        call .wait_data
        call .read_sector
        pop cx
        loop .read_block
    ret
    .wait_data:
        mov dx, 0x1f7
        .wait_check:
            in al, dx
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .wait_check
        ret
    .read_sector:
        mov dx, 0x1f0
        mov cx, 256
        .read_byte:
            in ax, dx
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .read_byte
        ret

write_disk:
    ; 设置读写扇区数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx; 0x1f3 
    mov al, cl; 起始扇区0~7位
    out dx, al

    inc dx; 0x1f4
    shr ecx, 8; 起始扇区8~15位
    mov al, cl
    out dx, al

    inc dx; 0x1f5 
    shr ecx, 8; 起始扇区16~23位
    mov al, cl
    out dx, al
    
    inc dx; 0x1f6
    shr ecx, 8; 起始扇区23~27位
    and cl, 0b1111

    mov al, 0b1110_0000
    or al, cl
    out dx, al

    inc dx; 0x1f7
    mov al, 0x30
    out dx, al
    xor ecx, ecx
    mov cl, bl
    .write_block:
        push cx
        call .write_sector
        call .wait_data
        pop cx
        loop .write_block
    ret
    .wait_data:
        mov dx, 0x1f7
        .wait_check:
            in al, dx
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .wait_check
        ret
    .write_sector:
        mov dx, 0x1f0
        mov cx, 256
        .write_byte:
            mov ax, [edi]
            out dx, ax
            jmp $+2
            add edi, 2
            loop .write_byte
        ret

ards_count:
    dd 0
ards_buffer:

