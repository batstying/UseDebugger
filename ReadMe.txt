A simple debugger by myself, try to achieve these functions:
构建基本的调试框架，并实现以下功能：
1）基本的运行功能：
	i.单步步入	(t)
	ii.单步步过	(p)
	iii.运行	(g [地址])

2）基本的数据查看、修改功能：
	i.反汇编		(u [addr])
	ii.显示内存数据(d [addr])
	iii.查看寄存器	(r)
	iv.修改数据    (e [addr])
	v.查看模块    (lm)

3）一般断点相关功能：
	i.设置断点	(bp addr)
	ii.显示断点	(bpl)
	iii.禁用/激活断点(bpd/bpe id)
	iv.删除断点	(bpc [id])

	其中,id为bpl所罗列结果的序号。

4）硬件断点相关的功能：
	i.设置断点	(bh addr e|w|a	1|2|4）
	ii.显示断点	(bhl)
	iii.删除断点	(bhc [id])

	其中，e表示硬件执行断点，w硬件写入断点，a硬件访问断	点
	硬件断点的可选长度1，2，4，需要根据地址的对齐情况来	确定。



5）内存断点相关的功能
	i.实现多内存断点功能
	ii.设置断点		(bm  addr a|w len)
	iii.查看所有内存断点(bml)
	iv.查看分页内断点	(bmpl)
	v.删除断点		(bmc [id])

6）自动跟踪记录功能
	i.记录运行中执行的指令 (trace addrstart  addrend 	[modulename])

7）脚本功能
	i.导出本次运行所执行的所有命令	(es)
	ii.导入之前保存的命令，并进行执行  (ls)

8)其他功能
	i.退出	(q)
	ii.查看栈  (ss)  (show stack)
	iii.查看函数列表 (sf) (show functions)