.global main
main:
	li a0,0 #a0=0
	li a1,100 #a1=100
	li a2,0 #a2=0
SUM:
	addi a0,a0,1 #a0=a0+1
	add a2,a2,a0 #a2=a2+a0
	bne a1,a0,SUM #if(a2!=a1) goto SUM
end:
	j end
