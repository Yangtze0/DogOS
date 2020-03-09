## DogOS
参考书：《30天自制操作系统》    
编译环境：默认macOS，根据平台选择中间目标文件类型（macho/elf/win）    
编译工具：gcc、nasm    
调试器：bochsdbg    

#### 使用方法

```
$ make
$ make run
```
make制作软盘映像文件dogos.img，作为bochs或其它虚拟机启动盘使用。


