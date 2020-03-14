## DogOS
一个真正从零开始的操作系统编码实践。    
参考书《30天自制操作系统》，相比较原书作者自制的很多工具和文件格式，本实现简化了代码和编译流程。    

编译环境：默认macOS，可选中间目标文件类型（macho/elf/win）    
编译工具：gcc、nasm    
调试器：bochsdbg    

#### 使用方法

```
$ make
$ make run
```
make制作软盘映像文件dogos.img，作为bochs或其它虚拟机启动盘使用。


