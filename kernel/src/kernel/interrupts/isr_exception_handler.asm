section .text
    global isr_exception_handler

isr_exception_handler:
    ; Register sichern
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Interruptnummer (über rdi) in al speichern
    mov al, byte [rdi]

    ; ISR-Handler aufrufen
    extern isr_handler
    call isr_handler

    ; Register wiederherstellen
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx

    ; Interrupt zurückgeben
    iret
