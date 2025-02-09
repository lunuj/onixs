# 启动文件
boot.asm

## 编译
nasm boot.asm -o boot.bin

## 创建硬盘镜像
bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat master.img

## 配置bochs
ata0-master: type=disk, path="master.img", mode=flat

## 写入硬盘镜像
dd if=boot.bin of=master.img bs=512 count=1 conv=notruncs

# 实模式
## 屏幕显示
1. 操作现存区域0xd8000，0xb8000 文本显示器的内存区域
2. 通过BIOS的int 0x10指令，向ah写入0x0e，再向al中写入文字，之后执行int 0x10指令

## 读写硬盘
通过外围端口对硬盘进行读写

# 加载程序
## 内核加载器 loader
### loader写入硬盘
	dd if=loader.bin of=master.img bs=512 count=4 seek=2 conv=notrunc
### loader读入内存
### 校验正确性，跳转至loader执行
## 内存检测

# 保护模式
## 全局描述符
## 全局描述符表
## A20线