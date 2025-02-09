[org 0x7c00]

; 设置屏幕模式为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00

call booting

mov edi, 0x1000; 读取的目标内存
mov ecx, 2; 起始扇区
mov bl, 4; 读写扇区数量

call read_disk
cmp word [0x1000], 0x55AA
jnz error
jmp 0:0x1002

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

msg_boot:
    db "Check dsik...", 10, 13, 0; \n \r
msg_error:
    db "Loader error", 10 ,13, 0;

booting:
    mov si, msg_boot
    call print
    ret
error:
    mov si, msg_error
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

; 填充 0
times 510 - ($ - $$) db 0

; 主引导扇区最后两个字节必须是 0x55 0xaa
dw 0xAA55