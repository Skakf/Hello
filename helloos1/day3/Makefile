TOOLPATH = ../z_tools/
MAKE	 = $(TOOLPATH)make.exe -r
NASK	 = $(TOOLPATH)nask.exe
EDIMG	 = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY	 = copy
DEL      = del

#文件生成规则

default :
    $(MAKE) img

#文件生成规则

ipl.bin : ipl.nas Makefile
    $(NASK) ipl.nas ipl.bin ipl.lst

haribote.sys : haribote.nas Makefile
    $(NASK) haribote.nas haribote.sys haribote.lst
    
haribote.img : ipl.bin haribote.sys Makefile
    $(EDIMG) imgin:../z_tools/fdimg0at.tek  wbinimg src:ipl.bin len:512 from:0 to:0 copy from:haribote.sys to:@: imgout:haribote.img
#asm :
#	$(MAKE) ipl.bin




img :
    $(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a:haribote.img

clean : 
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst
    -$(DEL) haribote.sys
    -$(DEL) haribote.lst
src_only :
    $(MAKE) clean
	-$(DEL) haribote.img

