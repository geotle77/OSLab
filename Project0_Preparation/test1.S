 .global main
main:
    li a0,3    #计数器
    li a2,100   #MAX

isprime:
  li a1, 0   #a1值为1表示质数，否则不为质数
  li t0, 2   #2-a0内的因子计数器

loop:
  beq t0, a0, done #如果计数器大于a0，则退出循环
  rem a3, a0, t0   #a3为a0除以计数器t0的余数
  beqz a3, not_prime #如果余数为0，则不是质数，跳转
  addi t0, t0, 1  #计数器加1
  j loop  #继续循环

not_prime:
  li a1, 0  #a1值为0表示不是质数，否则为质数
  addi a0,a0,1 #计数器加1
  bne a0,a2,isprime #如果计数器小于a2，则继续循环
  j end #结束

done:
  li a1,1 #质数
  addi a0,a0,1 #计数器加1
  bne a0,a2,isprime #如果计数器小于a2，则继续循环

end:
    j end
