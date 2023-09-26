
test:     file format elf64-littleriscv


Disassembly of section .text:

0000000050000000 <_ftext>:
    50000000:	4501                	li	a0,0
    50000002:	06400593          	li	a1,100
    50000006:	4601                	li	a2,0

0000000050000008 <SUM>:
    50000008:	0505                	addi	a0,a0,1
    5000000a:	962a                	add	a2,a2,a0
    5000000c:	feb61ee3          	bne	a2,a1,50000008 <SUM>

0000000050000010 <end>:
    50000010:	a001                	j	50000010 <end>
