# RuneOS

Um pequeno sistema operacional x86 de 32 bits, escrito do zero em C e NASM.

## Recursos (v0.1.0)
- Setor de boot de 512 bytes: carrega o kernel pela BIOS (LBA), ativa a linha A20 e entra em modo protegido
- GDT plana, IDT com tratamento de exceções da CPU (tela de kernel panic) e PIC 8259 remapeado
- Timer PIT (100 Hz) com uptime e sleep
- Driver de teclado PS/2 (layout US, Shift, Caps Lock)
- Console VGA em modo texto com rolagem e cursor de hardware (espelhado na serial COM1)
- Sistema de arquivos em RAM (16 arquivos, sem diretórios) e tabela de processos (ainda sem escalonador)
- Shell interativo: `help`, `ls`, `cat`, `write`, `ps`, `uptime`, `color`, `reboot` ...

## Compilar e executar
Requisitos: `gcc` (com suporte a 32 bits), `nasm`, `ld`, `objcopy`, `make` e `qemu-system-i386`.

    make          # gera build/os.img
    make run      # inicia no QEMU
    make run-serial   # igual, mas espelha o console no terminal

Se o kernel passar de 32 KiB, aumente `KERNEL_SECTORS` no Makefile.

## Rodar em outras VMs
A imagem `build/os.img` é um disco bruto com boot legado (BIOS). Em VirtualBox, VMware ou Hyper-V (Geração 1), converta a imagem para o formato de disco da VM e use firmware BIOS, não UEFI.

## Estrutura
    boot/boot.asm            setor de boot
    kernel/src/              ponto de entrada + kmain
    kernel/drivers/          VGA, serial, PIC, IDT/ISR/IRQ, relógio PIT, teclado, E/S de portas
    kernel/lib/src/          kstring, kstdio (kprintf)
    kernel/{tty,fs,proc,init}/
    kernel/shell/            tokenizador, interpretador, tabela de comandos
    kernel/include/          headers
    linker.ld                kernel linkado em 0x10000

## Roadmap
Setas e histórico de comandos, paginação + heap, escalonador de verdade, sistema de arquivos em disco, modo usuário.
