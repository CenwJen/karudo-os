.code16
.text

.global _start_setup
_start_setup:
    movw %cs, %ax
    movw %ax, %ds
    movw %ax, %es

    movw $setup_msg, %ax
    movw %ax, %bp
    movw $0x1301, %ax
    movw $0x000c, %bx
    movw $(setup_msg_end - setup_msg), %cx
    movw $0x0300, %dx
    int $0x10

loop:
    jmp loop

setup_msg:
    .ascii "setup is running"
setup_msg_end:
