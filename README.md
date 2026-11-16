# RuneOS

Um pequeno sistema operacional x86 de 32 bits, escrito do zero em C e NASM.

## Recursos
- Setor de boot de 512 bytes: carrega o kernel pela BIOS (LBA), ativa a linha A20 e entra em modo protegido
- GDT plana, IDT com tratamento de exceções da CPU (tela de kernel panic) e PIC 8259 remapeado
- Timer PIT (100 Hz) com uptime e sleep
- Driver de teclado PS/2 (layout US, Shift, Caps Lock e setas)
- Console VGA em modo texto com rolagem e cursor de hardware (espelhado na serial COM1)
- Sistema de arquivos em RAM com diretórios (128 entradas, arquivos de até 4 KB, 128 KB de dados) e tabela de processos (ainda sem escalonador)
- Gerenciador de arquivos com comandos de shell e modo tela cheia (`fm`)
- Shell interativo: `help`, `fm`, `ls`, `cd`, `cat`, `write`, `ps`, `uptime`, `color`, `reboot` ...

## Compilar e executar
Requisitos: `gcc` (com suporte a 32 bits), `nasm`, `ld`, `objcopy`, `make` e `qemu-system-i386`.

    make          # gera build/os.img
    make run      # inicia no QEMU
    make run-serial   # igual, mas espelha o console no terminal

Se o kernel passar de 48 KiB, aumente `KERNEL_SECTORS` no Makefile (hoje 96 setores de 512 bytes).

## Rodar em outras VMs
A imagem `build/os.img` é um disco bruto com boot legado (BIOS). Em VirtualBox, VMware ou Hyper-V (Geração 1), converta a imagem para o formato de disco da VM e use firmware BIOS, não UEFI.

## Gerenciador de arquivos
O sistema de arquivos (`kernel/fs/ramfs.c`) guarda tudo na RAM, então os arquivos somem ao reiniciar.
Limites: 128 entradas (arquivos e pastas), nomes de até 28 caracteres, 4 KB por arquivo e 128 KB de dados no total. Dá para ajustar tudo em `kernel/include/ramfs.h`.

Comandos do shell:

| Comando | O que faz |
|---|---|
| `ls [-l] [dir]` | lista o conteúdo (pastas primeiro) |
| `cd [dir]` / `pwd` | muda de pasta / mostra a pasta atual |
| `tree [dir]` | mostra a árvore de pastas |
| `mkdir <dir>` / `touch <arq>` | cria pasta / arquivo vazio |
| `cat <arq>` | mostra o conteúdo |
| `write <arq> <texto...>` / `append <arq> <texto...>` | substitui / acrescenta texto |
| `cp <origem> <destino>` | copia um arquivo |
| `mv <origem> <destino>` | move ou renomeia |
| `rm [-r] <alvo>` | apaga (`-r` para pastas com conteúdo) |
| `df` | mostra o espaço livre |
| `fm` | abre o gerenciador em tela cheia |

Os caminhos aceitam `/absoluto`, `relativo`, `.` e `..`.

Modo tela cheia (`fm`):

| Tecla | Ação |
|---|---|
| setas ou `w`/`s` | mover a seleção |
| `Enter` | abrir pasta ou ver arquivo |
| `Backspace` | subir para a pasta de cima |
| `n` / `m` | novo arquivo / nova pasta |
| `e` | substituir o texto do arquivo (uma linha) |
| `r` | renomear |
| `c` | copiar arquivo |
| `d` | apagar (pede confirmação com `y`) |
| `q` ou `Esc` | sair |

Teste no PC, sem precisar do QEMU (a partir da raiz do projeto):

    gcc -std=c99 -Wall -Wextra -Ikernel/include kernel/fs/test_host.c kernel/fs/fm.c kernel/fs/ramfs.c -o t && ./t

O `ramfs` usa só a API `rfs_*` (`kernel/include/ramfs.h`), então dá para trocá-lo por um sistema de arquivos em disco mantendo os comandos como estão. O `fs.c` original (lista simples de arquivos) continua no projeto, mas nenhum comando usa mais ele.

## Estrutura
    boot/boot.asm            setor de boot
    kernel/src/              ponto de entrada + kmain
    kernel/drivers/          VGA, serial, PIC, IDT/ISR/IRQ, relógio PIT, teclado, E/S de portas
    kernel/lib/src/          kstring, kstdio (kprintf)
    kernel/fs/               ramfs (arquivos e pastas), fm (gerenciador), fm_glue (liga ao VGA/teclado), fs (antigo)
    kernel/{tty,proc,init}/
    kernel/shell/            tokenizador, interpretador, tabela de comandos
    kernel/include/          headers
    linker.ld                kernel linkado em 0x10000

## Roadmap
Histórico de comandos e setas no prompt do shell, editor de texto, paginação + heap, escalonador de verdade, sistema de arquivos em disco, modo usuário.
