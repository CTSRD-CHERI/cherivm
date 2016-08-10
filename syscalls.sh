for I in Syscall_{cheri,mips,cheri_security_manager} ; do
	echo $I
	cut -d , -f 2 $I | tail +2 > $I.cycles
done
ministat Syscall_mips.cycles Syscall_cheri.cycles Syscall_cheri_security_manager.cycles

